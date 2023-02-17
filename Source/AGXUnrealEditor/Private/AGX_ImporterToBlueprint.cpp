// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ImporterToBlueprint.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImportEnums.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_ModelSourceComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SimObjectsImporterHelper.h"
#include "AGXSimObjectsReader.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "SimulationObjectCollection.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_PropertyUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "AssetSelection.h"
#include "AssetToolsModule.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/ScopedSlowTask.h"
#include "PackageTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "AGX_ImporterToBlueprint"

namespace
{
	void PreCreationSetup()
	{
		GEditor->SelectNone(false, false);
	}

	FString CreateBlueprintPackagePath(FAGX_SimObjectsImporterHelper& Helper, bool IsBase)
	{
		// Create directory for this import and a "Blueprints" directory inside of that.
		/// \todo I think this is more complicated than it needs to be. What are all the pieces for?
		const FString SubDirectory = IsBase ? "Blueprint" : "";
		FString ParentPackagePath =
			FAGX_ImportUtilities::CreatePackagePath(Helper.DirectoryName, SubDirectory);
		FGuid BaseNameGuid = FGuid::NewGuid();
		FString ParentAssetName = IsBase ? BaseNameGuid.ToString() : Helper.SourceFileName;

		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(ParentPackagePath, ParentAssetName);
		UPackage* ParentPackage = CreatePackage(*ParentPackagePath);
		FString Path = FPaths::GetPath(ParentPackage->GetName());

		UE_LOG(
			LogAGX, Display, TEXT("File '%s' imported to package '%s', path '%s'"),
			*Helper.SourceFileName, *ParentPackagePath, *Path);

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName =
			IsBase ? "BP_Base_" + BaseNameGuid.ToString() : TEXT("BP_") + Helper.DirectoryName;
		FString BasePackagePath = UPackageTools::SanitizePackageName(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		auto PackageExists = [](const FString& PackagePath)
		{
			check(!FEditorFileUtils::IsMapPackageAsset(PackagePath));
			return FPackageName::DoesPackageExist(PackagePath) ||
				   FindObject<UPackage>(nullptr, *PackagePath) != nullptr;
		};

		/// \todo Should be possible to use one of the unique name creators here.
		int32 TryCount = 0;
		while (PackageExists(PackagePath))
		{
			++TryCount;
			PackagePath = BasePackagePath + "_" + FString::FromInt(TryCount);
		}

		return PackagePath;
	}

	UPackage* GetPackage(const FString& BlueprintPackagePath)
	{
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *BlueprintPackagePath);
#else
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
#endif
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	bool AddShapeMaterials(
		const FSimulationObjectCollection& SimObjects, FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& ShapeMaterial : SimObjects.GetShapeMaterials())
		{
			Success &= Helper.InstantiateShapeMaterial(ShapeMaterial) != nullptr;
		}

		return Success;
	}

	bool AddContactMaterials(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		UAGX_ContactMaterialRegistrarComponent* CMRegistrar =
			SimObjects.GetContactMaterials().Num() > 0
				? Helper.InstantiateContactMaterialRegistrar(ImportedActor)
				: nullptr;
		if (CMRegistrar == nullptr)
		{
			// Imported model has no ContactMaterials, we are done.
			return true;
		}

		bool Success = true;
		for (const auto& ContactMaterial : SimObjects.GetContactMaterials())
		{
			Success &= Helper.InstantiateContactMaterial(ContactMaterial, *CMRegistrar) != nullptr;
		}

		return Success;
	}

	bool AddBodilessShapes(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Shape : SimObjects.GetSphereShapes())
		{
			Success &= Helper.InstantiateSphere(Shape, ImportedActor) != nullptr;
		}

		for (const auto& Shape : SimObjects.GetBoxShapes())
		{
			Success &= Helper.InstantiateBox(Shape, ImportedActor) != nullptr;
		}

		for (const auto& Shape : SimObjects.GetCylinderShapes())
		{
			Success &= Helper.InstantiateCylinder(Shape, ImportedActor) != nullptr;
		}

		for (const auto& Shape : SimObjects.GetCapsuleShapes())
		{
			Success &= Helper.InstantiateCapsule(Shape, ImportedActor) != nullptr;
		}

		for (const auto& Shape : SimObjects.GetTrimeshShapes())
		{
			if (Helper.bIgnoreDisabledTrimeshes && !Shape.GetEnableCollisions())
			{
				if (Shape.HasRenderData())
				{
					Success &=
						Helper.InstantiateRenderDataInBodyOrRoot(Shape, ImportedActor) != nullptr;
				}
			}
			else
			{
				Success &= Helper.InstantiateTrimesh(Shape, ImportedActor) != nullptr;
			}
		}

		return Success;
	}

	bool AddRigidBodyAndAnyOwnedShape(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Body : SimObjects.GetRigidBodies())
		{
			Success &= Helper.InstantiateBody(Body, ImportedActor) != nullptr;

			for (const auto& Sphere : Body.GetSphereShapes())
			{
				Success &= Helper.InstantiateSphere(Sphere, ImportedActor, &Body) != nullptr;
			}

			for (const auto& Box : Body.GetBoxShapes())
			{
				Success &= Helper.InstantiateBox(Box, ImportedActor, &Body) != nullptr;
			}

			for (const auto& Capsule : Body.GetCapsuleShapes())
			{
				Success &= Helper.InstantiateCapsule(Capsule, ImportedActor, &Body) != nullptr;
			}

			for (const auto& Cylinder : Body.GetCylinderShapes())
			{
				Success &= Helper.InstantiateCylinder(Cylinder, ImportedActor, &Body) != nullptr;
			}

			for (const auto& Trimesh : Body.GetTrimeshShapes())
			{
				if (Helper.bIgnoreDisabledTrimeshes && !Trimesh.GetEnableCollisions())
				{
					if (Trimesh.HasRenderData())
					{
						Success &= Helper.InstantiateRenderDataInBodyOrRoot(
									   Trimesh, ImportedActor, &Body) != nullptr;
					}
				}
				else
				{
					Success &= Helper.InstantiateTrimesh(Trimesh, ImportedActor, &Body) != nullptr;
				}
			}
		}

		return Success;
	}

	bool AddTracks(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Track : SimObjects.GetTracks())
		{
			Success &= Helper.InstantiateTrack(Track, ImportedActor) != nullptr;
		}

		return Success;
	}

	bool AddTireModels(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Tire : SimObjects.GetTwoBodyTires())
		{
			check(Tire.GetTireRigidBody().HasNative());
			check(Tire.GetHubRigidBody().HasNative());
			Helper.InstantiateTwoBodyTire(Tire, ImportedActor);
		}

		return Success;
	}

	bool AddConstraints(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Constraint : SimObjects.GetHingeConstraints())
		{
			auto Hinge = Helper.InstantiateHinge(Constraint, ImportedActor);
			Success &= Hinge != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetPrismaticConstraints())
		{
			auto Prismatic = Helper.InstantiatePrismatic(Constraint, ImportedActor);
			Success &= Prismatic != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetBallConstraints())
		{
			auto BallConstraint = Helper.InstantiateBallConstraint(Constraint, ImportedActor);
			Success &= BallConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetCylindricalConstraints())
		{
			auto CylindricalConstraint =
				Helper.InstantiateCylindricalConstraint(Constraint, ImportedActor);
			Success &= CylindricalConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetDistanceConstraints())
		{
			auto DistanceConstraint =
				Helper.InstantiateDistanceConstraint(Constraint, ImportedActor);
			Success &= DistanceConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetLockConstraints())
		{
			auto LockConstraint = Helper.InstantiateLockConstraint(Constraint, ImportedActor);
			Success &= LockConstraint != nullptr;
		}

		return Success;
	}

	bool AddDisabledCollisionGroups(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		const auto DisabledGroups = SimObjects.GetDisabledCollisionGroups();
		if (DisabledGroups.Num() == 0)
		{
			return true;
		}

		return Helper.InstantiateCollisionGroupDisabler(ImportedActor, DisabledGroups) != nullptr;
	}

	bool AddWires(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& Wire : SimObjects.GetWires())
		{
			Success &= Helper.InstantiateWire(Wire, ImportedActor) != nullptr;
		}

		return Success;
	}

	bool AddObserverFrames(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		bool Success = true;
		for (const auto& ObserverFr : SimObjects.GetObserverFrames())
		{
			Success &= Helper.InstantiateObserverFrame(
						   ObserverFr.Name, ObserverFr.BodyGuid, ObserverFr.ObserverGuid,
						   ObserverFr.Transform, ImportedActor) != nullptr;
		}

		return Success;
	}

	bool AddAllComponents(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		FScopedSlowTask ImportTask(100.f, LOCTEXT("ImportModel", "Importing model"), true);
		ImportTask.MakeDialog();
		bool Success = true;

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Shape Materials"));
		Success &= AddShapeMaterials(SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Contact Materials"));
		Success &= AddContactMaterials(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(
			5.f, FText::FromString("Reading Rigid Bodies and their Shapes"));
		Success &= AddRigidBodyAndAnyOwnedShape(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(10.f, FText::FromString("Reading Bodiless Shapes"));
		Success &= AddBodilessShapes(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Tracks"));
		Success &= AddTracks(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Tire Models"));
		Success &= AddTireModels(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Constraints"));
		Success &= AddConstraints(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Disabled Collision Groups"));
		Success &= AddDisabledCollisionGroups(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Wires"));
		Success &= AddWires(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Reading Observer Frames"));
		Success &= AddObserverFrames(ImportedActor, SimObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Finalizing Import"));
		Helper.InstantiateModelSourceComponent(ImportedActor);
		Helper.FinalizeImport();

		ImportTask.EnterProgressFrame(40.f, FText::FromString("Import complete"));
		return Success;
	}

	bool AddComponentsFromAGXArchive(AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper)
	{
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadAGXArchive(Helper.SourceFilePath, SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			return false;
		}

		return true;
	}

	bool AddComponentsFromUrdf(
		AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_ImportSettings& ImportSettings)
	{
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadUrdf(
				ImportSettings.FilePath, ImportSettings.UrdfPackagePath, SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			return false;
		}

		return true;
	}

	AActor* CreateTemplate(
		FAGX_SimObjectsImporterHelper& Helper, const FAGX_ImportSettings& ImportSettings)
	{
		UActorFactory* Factory =
			GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
		FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
		UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
		AActor* RootActorContainer =
			FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
		check(RootActorContainer != nullptr); /// \todo Test and return false instead of check?
		RootActorContainer->SetFlags(RF_Transactional);
		RootActorContainer->SetActorLabel(Helper.DirectoryName);

// I would like to be able to create and configure the RootComponent here, but
// the way Blueprint creation has been done in Unreal Engine makes this
// impossible. A new RootComponent is always created and the DefaultSceneRoot I
// create here is made a child of that new SceneComponent. Not what I want. My
// work-around for now is to rely on the implicitly created RootComponent and
// hoping it does what we want in all cases. I leave SceneComponents that should
// be attached to the RootComponent unconnected, they are implicitly connected
// to the implicit RootComponent by the Blueprint creator code. This produces a
// weird/invalid template actor so I'm worried that the it-happens-to-work state
// we now have won't survive for long.
#if 0
		USceneComponent* ActorRootComponent = NewObject<USceneComponent>(
			RootActorContainer, USceneComponent::GetDefaultSceneRootVariableName());
		check(ActorRootComponent != nullptr);
		ActorRootComponent->Mobility = EComponentMobility::Movable;
		ActorRootComponent->bVisualizeComponent = true;
		ActorRootComponent->SetFlags(RF_Transactional);
		ActorRootComponent->RegisterComponent();
		RootActorContainer->AddInstanceComponent(ActorRootComponent);
		RootActorContainer->SetRootComponent(ActorRootComponent);
#endif

		const bool Result = ImportSettings.ImportType == EAGX_ImportType::Urdf
								? AddComponentsFromUrdf(*RootActorContainer, Helper, ImportSettings)
								: AddComponentsFromAGXArchive(*RootActorContainer, Helper);

		if (!Result)
		{
			/// @todo Is there some clean-up I need to do for RootActorContainer and/or
			/// EmptyActorAsset here? I tried with MarkPendingKill but that caused
			///    Assertion failed: !IsRooted()
			// RootActorContainer->MarkPendingKill();
			// EmptyActorAsset->MarkPendingKill();
			return nullptr;
		}

		// Should the EmptyActorAsset be cleaned up somehow? MarkPendingKill?
		return RootActorContainer;
	}

	UBlueprint* CreateBaseBlueprint(UPackage* Package, AActor* Template)
	{
		static constexpr bool ReplaceInWorld = false;
		static constexpr bool KeepMobility = true;
		FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
		Params.bReplaceActor = ReplaceInWorld;
		Params.bKeepMobility = KeepMobility;
		Params.bOpenBlueprint = false;

		UBlueprint* Blueprint =
			FKismetEditorUtilities::CreateBlueprintFromActor(Package->GetName(), Template, Params);
		check(Blueprint);
		return Blueprint;
	}

	void PostCreationTeardown(
		AActor* Template, UPackage* Package, UBlueprint* Blueprint, const FString& PackagePath)
	{
		if (Template != nullptr)
		{
			Template->Destroy();
		}

		GEngine->BroadcastLevelActorListChanged();

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());

#if UE_VERSION_OLDER_THAN(5, 0, 0)
		UPackage::SavePackage(
			Package, Blueprint, RF_Public | RF_Standalone, *PackageFilename, GError, nullptr, true,
			true, SAVE_NoError);
#else
		FSavePackageArgs SaveArgs;
		// SaveArgs.TargetPlatform = ???; // I think we can leave this at the default: not cooking.
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		// SaveArgs.SaveFlags = ???; // I think we can leave this at the default: None.
		// SaveArgs.bForceByteSwapping = ???; // I think we can leave this at the default: false.
		// SaveArgs.bWarnOfLongFilename = ???; // I think we can leave this at the default: true.
		// SaveArgs.bSlowTask = ???; // I think we can leave this at the default: true.
		// SaveArgs.Error = ???; // I think we can leave this at the default: GError.
		// SaveArgs.SavePAckageContext = ???; // I think we can leave this at the default: nullptr.
		UPackage::SavePackage(Package, Blueprint, *PackageFilename, SaveArgs);
#endif
	}

	UBlueprint* ImportToBaseBlueprint(
		FAGX_SimObjectsImporterHelper& Helper, const FAGX_ImportSettings& ImportSettings)
	{
		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Helper, true);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		AActor* Template = CreateTemplate(Helper, ImportSettings);
		if (Template == nullptr)
		{
			return nullptr;
		}

		UBlueprint* Blueprint = CreateBaseBlueprint(Package, Template);
		PostCreationTeardown(Template, Package, Blueprint, BlueprintPackagePath);
		return Blueprint;
	}

	UBlueprint* CreateChildBlueprint(
		UBlueprint* BaseBlueprint, FAGX_SimObjectsImporterHelper& Helper)
	{
		if (BaseBlueprint == nullptr)
		{
			return nullptr;
		}

		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Helper, false);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		const FString AssetName = FPaths::GetBaseFilename(Package->GetName());

		UBlueprint* BlueprintChild = FKismetEditorUtilities::CreateBlueprint(
			BaseBlueprint->GeneratedClass, Package, FName(AssetName), EBlueprintType::BPTYPE_Normal,
			UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(),
			FName("AGXUnrealImport"));

		PostCreationTeardown(nullptr, Package, BlueprintChild, BlueprintPackagePath);
		return BlueprintChild;
	}

	UBlueprint* ImportToBlueprint(
		FAGX_SimObjectsImporterHelper& Helper, const FAGX_ImportSettings& ImportSettings)
	{
		// The result of the import is stored in the BlueprintBase which is placed in the
		// 'Blueprint' directory in the context browser and should never be edited by the user. It
		// is the "original". The BlueprintChild is what the user will interact directly with, and
		// it is a child of the BlueprintBase. This way, we can ensure model synchronization works
		// as intended.
		UBlueprint* BlueprintBase = ImportToBaseBlueprint(Helper, ImportSettings);
		if (BlueprintBase == nullptr)
		{
			return nullptr;
		}

		UBlueprint* BlueprintChild = CreateChildBlueprint(BlueprintBase, Helper);

		if (BlueprintChild != nullptr && ImportSettings.bOpenBlueprintEditorAfterImport)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(
				BlueprintChild);
		}

		return BlueprintChild;
	}
}

UBlueprint* AGX_ImporterToBlueprint::Import(const FAGX_ImportSettings& ImportSettings)
{
	FAGX_SimObjectsImporterHelper Helper(
		ImportSettings.FilePath, ImportSettings.bIgnoreDisabledTrimeshes);
	UBlueprint* Bp = ImportToBlueprint(Helper, ImportSettings);
	if (Bp == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Some issues occurred during import. Log category LogAGX in the Console may "
			"contain more information.",
			"Import model to Blueprint");
	}

	return Bp;
}

namespace AGX_ImporterToBlueprint_SynchronizeModel_helpers
{
	template <typename T>
	TArray<FGuid> GetGuidsFromBarriers(const TArray<T>& Barriers)
	{
		TArray<FGuid> Guids;
		Guids.Reserve(Barriers.Num());
		for (const auto& Barrier : Barriers)
		{
			Guids.Add(Barrier.GetGuid());
		}

		return Guids;
	}

	template <typename T>
	TMap<FGuid, T*> CreateGuidToComponentMap(const TArray<T*>& Components)
	{
		TMap<FGuid, T*> Map;
		Map.Reserve(Components.Num());
		for (T* Component : Components)
		{
			if (Component != nullptr)
				Map.Add(Component->ImportGuid, Component);
		}

		return Map;
	}

	template <typename T>
	TMap<FGuid, T*> FindAGXAssetComponents(const FString& AssetDirPath)
	{
		TArray<T*> FoundAssets = FAGX_EditorUtilities::FindAssets<T>(AssetDirPath);
		return CreateGuidToComponentMap<T>(FoundAssets);
	}

	FString GetImportDirPath(
		const FAGX_SimObjectsImporterHelper& Helper, const FString& Subdir = "")
	{
		return FPaths::Combine(
			FString("/Game"), FAGX_ImportUtilities::GetImportRootDirectoryName(),
			Helper.DirectoryName, Subdir);
	}

	struct FShapeGuidsCollection
	{
		// Values of the TMaps indicates collision enabled status.
		TMap<FGuid, bool> SphereShapeGuids;
		TMap<FGuid, bool> BoxShapeGuids;
		TMap<FGuid, bool> CylinderShapeGuids;
		TMap<FGuid, bool> CapsuleShapeGuids;
		TMap<FGuid, bool> TrimeshShapeGuids;
		TArray<FGuid> RenderDataGuids;
	};

	FShapeGuidsCollection GetShapeGuids(const FSimulationObjectCollection& SimulationObjects)
	{
		FShapeGuidsCollection Infos;

		auto GetRenderDataGuidFrom = [](const FShapeBarrier& ShapeBarrier) -> TOptional<FGuid>
		{
			if (ShapeBarrier.HasValidRenderData())
			{
				FRenderDataBarrier Rd = ShapeBarrier.GetRenderData();
				return Rd.GetGuid();
			}

			return {};
		};

		// Iterate all Shapes owned by a Rigid Body.
		for (const auto& Body : SimulationObjects.GetRigidBodies())
		{
			for (const auto& Shape : Body.GetSphereShapes())
			{
				Infos.SphereShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetBoxShapes())
			{
				Infos.BoxShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetCylinderShapes())
			{
				Infos.CylinderShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetCapsuleShapes())
			{
				Infos.CapsuleShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetTrimeshShapes())
			{
				Infos.TrimeshShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
		}

		// Iterate all "free" Shapes, not owned by a Rigid Body.
		for (const auto& Barrier : SimulationObjects.GetSphereShapes())
		{
			Infos.SphereShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetBoxShapes())
		{
			Infos.BoxShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetCylinderShapes())
		{
			Infos.CylinderShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetCapsuleShapes())
		{
			Infos.CapsuleShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetTrimeshShapes())
		{
			Infos.TrimeshShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.TrimeshShapeGuids.Add(*RenderDataGuid);
		}

		return Infos;
	}

	struct SCSNodeCollection
	{
		SCSNodeCollection(const UBlueprint& Bp)
		{
			if (Bp.SimpleConstructionScript == nullptr)
			{
				return;
			}

			for (USCS_Node* Node : Bp.SimpleConstructionScript->GetAllNodes())
			{
				if (Node == nullptr)
				{
					continue;
				}

				UActorComponent* Component = Node->ComponentTemplate;
				if (Component == nullptr)
				{
					continue;
				}

				if (Component ==
					Bp.SimpleConstructionScript->GetDefaultSceneRootNode()->ComponentTemplate)
				{
					AGX_CHECK(RootComponent == nullptr);
					RootComponent = Node;
				}
				else if (auto Ri = Cast<UAGX_RigidBodyComponent>(Component))
				{
					AGX_CHECK(!RigidBodies.Contains(Ri->ImportGuid));
					if (Ri->ImportGuid.IsValid())
						RigidBodies.Add(Ri->ImportGuid, Node);
				}
				else if (auto Sph = Cast<UAGX_SphereShapeComponent>(Component))
				{
					AGX_CHECK(!SphereShapes.Contains(Sph->ImportGuid));
					if (Sph->ImportGuid.IsValid())
						SphereShapes.Add(Sph->ImportGuid, Node);
				}
				else if (auto Box = Cast<UAGX_BoxShapeComponent>(Component))
				{
					AGX_CHECK(!BoxShapes.Contains(Box->ImportGuid));
					if (Box->ImportGuid.IsValid())
						BoxShapes.Add(Box->ImportGuid, Node);
				}
				else if (auto Cyl = Cast<UAGX_CylinderShapeComponent>(Component))
				{
					AGX_CHECK(!CylinderShapes.Contains(Cyl->ImportGuid));
					if (Cyl->ImportGuid.IsValid())
						CylinderShapes.Add(Cyl->ImportGuid, Node);
				}
				else if (auto Cap = Cast<UAGX_CapsuleShapeComponent>(Component))
				{
					AGX_CHECK(!CapsuleShapes.Contains(Cap->ImportGuid));
					if (Cap->ImportGuid.IsValid())
						CapsuleShapes.Add(Cap->ImportGuid, Node);
				}
				else if (auto Tri = Cast<UAGX_TrimeshShapeComponent>(Component))
				{
					AGX_CHECK(!TrimeshShapes.Contains(Tri->ImportGuid));
					if (Tri->ImportGuid.IsValid())
						TrimeshShapes.Add(Tri->ImportGuid, Node);
				}
				else if (auto Co = Cast<UAGX_ConstraintComponent>(Component))
				{
					if (auto Hi = Cast<UAGX_HingeConstraintComponent>(Component))
					{
						AGX_CHECK(!HingeConstraints.Contains(Hi->ImportGuid));
						if (Hi->ImportGuid.IsValid())
							HingeConstraints.Add(Hi->ImportGuid, Node);
					}
					else if (auto Pr = Cast<UAGX_PrismaticConstraintComponent>(Component))
					{
						AGX_CHECK(!PrismaticConstraints.Contains(Pr->ImportGuid));
						if (Pr->ImportGuid.IsValid())
							PrismaticConstraints.Add(Pr->ImportGuid, Node);
					}
					else if (auto Ba = Cast<UAGX_BallConstraintComponent>(Component))
					{
						AGX_CHECK(!BallConstraints.Contains(Ba->ImportGuid));
						if (Ba->ImportGuid.IsValid())
							BallConstraints.Add(Ba->ImportGuid, Node);
					}
					else if (auto Cy = Cast<UAGX_CylindricalConstraintComponent>(Component))
					{
						AGX_CHECK(!CylindricalConstraints.Contains(Cy->ImportGuid));
						if (Cy->ImportGuid.IsValid())
							CylindricalConstraints.Add(Cy->ImportGuid, Node);
					}
					else if (auto Di = Cast<UAGX_DistanceConstraintComponent>(Component))
					{
						AGX_CHECK(!DistanceConstraints.Contains(Di->ImportGuid));
						if (Di->ImportGuid.IsValid())
							DistanceConstraints.Add(Di->ImportGuid, Node);
					}
					else if (auto Lo = Cast<UAGX_LockConstraintComponent>(Component))
					{
						AGX_CHECK(!LockConstraints.Contains(Lo->ImportGuid));
						if (Lo->ImportGuid.IsValid())
							LockConstraints.Add(Lo->ImportGuid, Node);
					}
					else
					{
						UE_LOG(
							LogAGX, Error, TEXT("Found Node: '%s' with unsupported type."),
							*Node->GetName());
						AGX_CHECK(false);
					}
				}
				else if (auto Re = Cast<UAGX_ModelSourceComponent>(Component))
				{
					AGX_CHECK(ModelSourceComponent == nullptr);
					ModelSourceComponent = Node;
					for (const auto& SMCTuple : Re->StaticMeshComponentToOwningTrimesh)
					{
						if (USCS_Node* StaticMeshComponentNode =
								Bp.SimpleConstructionScript->FindSCSNode(FName(SMCTuple.Key)))
						{
							const FGuid Guid = SMCTuple.Value;
							if (!Guid.IsValid())
								continue;

							AGX_CHECK(!CollisionStaticMeshComponents.Contains(Guid));
							CollisionStaticMeshComponents.Add(Guid, StaticMeshComponentNode);
						}
					}
					for (const auto& SMCTuple : Re->StaticMeshComponentToOwningRenderData)
					{
						if (USCS_Node* StaticMeshComponentNode =
								Bp.SimpleConstructionScript->FindSCSNode(FName(SMCTuple.Key)))
						{
							const FGuid Guid = SMCTuple.Value;
							if (!Guid.IsValid())
								continue;

							AGX_CHECK(!RenderStaticMeshComponents.Contains(Guid));
							RenderStaticMeshComponents.Add(Guid, StaticMeshComponentNode);
						}
					}
				}
				else if (auto St = Cast<UStaticMeshComponent>(Component))
				{
					// Handled by gathering information from the ModelSourceComponent since a Static
					// Mesh Component does not have an Import Guid.
				}
				else if (auto Con = Cast<UAGX_ContactMaterialRegistrarComponent>(Component))
				{
					AGX_CHECK(ContactMaterialRegistrarComponent == nullptr);
					ContactMaterialRegistrarComponent = Node;
				}
				else if (auto Col = Cast<UAGX_CollisionGroupDisablerComponent>(Component))
				{
					AGX_CHECK(CollisionGroupDisablerComponent == nullptr);
					CollisionGroupDisablerComponent = Node;
				}
				else if (auto Tw = Cast<UAGX_TwoBodyTireComponent>(Component))
				{
					AGX_CHECK(!TwoBodyTires.Contains(Tw->ImportGuid));
					if (Tw->ImportGuid.IsValid())
						TwoBodyTires.Add(Tw->ImportGuid, Node);
				}
				else if (auto Ob = Cast<UAGX_ObserverFrameComponent>(Component))
				{
					AGX_CHECK(!ObserverFrames.Contains(Ob->ImportGuid));
					if (Ob->ImportGuid.IsValid())
						ObserverFrames.Add(Ob->ImportGuid, Node);
				}
				else if (auto Wi = Cast<UAGX_WireComponent>(Component))
				{
					// Not supported, will be ignored.
				}
				else if (auto Tr = Cast<UAGX_TrackComponent>(Component))
				{
					// Not supported, will be ignored.
				}
				else
				{
					// We should never encounter a Component type that does not match any of the
					// above.
					UE_LOG(
						LogAGX, Warning, TEXT("Found Node: '%s' with unsupported type."),
						*Node->GetName());
					AGX_CHECK(false);
				}
			}
		}

		// The key is the AGX Dynamics object's UUID converted to an FGuid at the time of the
		// previous import.
		TMap<FGuid, USCS_Node*> RigidBodies;

		// Shapes are all Shapes, including Shapes owned by Rigid Bodies.
		TMap<FGuid, USCS_Node*> SphereShapes;
		TMap<FGuid, USCS_Node*> BoxShapes;
		TMap<FGuid, USCS_Node*> CylinderShapes;
		TMap<FGuid, USCS_Node*> CapsuleShapes;
		TMap<FGuid, USCS_Node*> TrimeshShapes;

		TMap<FGuid, USCS_Node*> HingeConstraints;
		TMap<FGuid, USCS_Node*> PrismaticConstraints;
		TMap<FGuid, USCS_Node*> BallConstraints;
		TMap<FGuid, USCS_Node*> CylindricalConstraints;
		TMap<FGuid, USCS_Node*> DistanceConstraints;
		TMap<FGuid, USCS_Node*> LockConstraints;
		TMap<FGuid, USCS_Node*> TwoBodyTires;
		TMap<FGuid, USCS_Node*> ObserverFrames;

		// Guid is the AGX Dynamics shape (Trimesh) guid.
		TMap<FGuid, USCS_Node*> CollisionStaticMeshComponents;

		// Guid is the AGX Dynamics RenderData guid.
		TMap<FGuid, USCS_Node*> RenderStaticMeshComponents;

		USCS_Node* CollisionGroupDisablerComponent = nullptr;
		USCS_Node* ContactMaterialRegistrarComponent = nullptr;
		USCS_Node* ModelSourceComponent = nullptr;
		USCS_Node* RootComponent = nullptr;
	};

	// Returns true if at least one Guid could be matched, false otherwise.
	bool EnsureSameSource(
		const SCSNodeCollection& SCSNodes, const FSimulationObjectCollection& SimulationObjects)
	{
		auto Contains = [](const TMap<FGuid, USCS_Node*>& OldNodes, const TArray<FGuid>& NewGuids)
		{
			for (auto& NodeTuple : OldNodes)
			{
				if (NewGuids.Contains(NodeTuple.Key))
				{
					return true;
				}
			}
			return false;
		};

		// Check against all Rigid Bodies.
		const TArray<FGuid> BodyBarrierGuids =
			GetGuidsFromBarriers(SimulationObjects.GetRigidBodies());
		if (Contains(SCSNodes.RigidBodies, BodyBarrierGuids))
		{
			return true;
		}

		// Check against all Shapes.
		const FShapeGuidsCollection NewShapeGuids = GetShapeGuids(SimulationObjects);
		TArray<FGuid> SphereShapeBarrierGuids;
		NewShapeGuids.SphereShapeGuids.GenerateKeyArray(SphereShapeBarrierGuids);
		if (Contains(SCSNodes.SphereShapes, SphereShapeBarrierGuids))
		{
			return true;
		}

		TArray<FGuid> BoxShapeBarrierGuids;
		NewShapeGuids.BoxShapeGuids.GenerateKeyArray(BoxShapeBarrierGuids);
		if (Contains(SCSNodes.BoxShapes, BoxShapeBarrierGuids))
		{
			return true;
		}

		TArray<FGuid> CylinderShapeBarrierGuids;
		NewShapeGuids.CylinderShapeGuids.GenerateKeyArray(CylinderShapeBarrierGuids);
		if (Contains(SCSNodes.CylinderShapes, CylinderShapeBarrierGuids))
		{
			return true;
		}

		TArray<FGuid> CapsuleShapeBarrierGuids;
		NewShapeGuids.CapsuleShapeGuids.GenerateKeyArray(CapsuleShapeBarrierGuids);
		if (Contains(SCSNodes.CapsuleShapes, CapsuleShapeBarrierGuids))
		{
			return true;
		}

		TArray<FGuid> TrimeshShapeBarrierGuids;
		NewShapeGuids.TrimeshShapeGuids.GenerateKeyArray(TrimeshShapeBarrierGuids);
		if (Contains(SCSNodes.TrimeshShapes, TrimeshShapeBarrierGuids))
		{
			return true;
		}

		return false;
	}

	void AddOrUpdateShapeMaterials(
		UBlueprint& BaseBP, const FSimulationObjectCollection& SimulationObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		// Find all existing assets that might be of interest from the previous import.
		const FString ShapeMaterialDirPath =
			GetImportDirPath(Helper, FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName());
		TMap<FGuid, UAGX_ShapeMaterial*> ExistingShapeMaterialsMap =
			FindAGXAssetComponents<UAGX_ShapeMaterial>(ShapeMaterialDirPath);

		// Synchronize the Shape Material assets, either updating existing ones with new data from
		// the simulation objects or creating brand new ones for materials we don't have assets
		// for. Asset deletion for materials removed from the simulation objects has already been
		// done by DeleteRemovedAssets.
		//
		// Regardless of whether we update and save or instantiate a Shape Material, the asset is
		// registered in RestoredShapeMaterials so that the Shapes that will be added or updated
		// shortly can find them.
		for (const auto& Barrier : SimulationObjects.GetShapeMaterials())
		{
			const FGuid Guid = Barrier.GetGuid();
			if (ExistingShapeMaterialsMap.Contains(Guid))
			{
				Helper.UpdateAndSaveShapeMaterialAsset(Barrier, *ExistingShapeMaterialsMap[Guid]);
			}
			else
			{
				Helper.InstantiateShapeMaterial(Barrier);
			}
		}
	}

	TArray<FAGX_RenderMaterial> CollectRenderMaterials(
		const FSimulationObjectCollection& SimulationObjects)
	{
		TArray<FAGX_RenderMaterial> Materials;
		auto CollectFromShapes = [&Materials](const auto& ShapeBarriers)
		{
			for (const auto& ShapeBarrier : ShapeBarriers)
			{
				if (!ShapeBarrier.HasRenderMaterial())
				{
					continue;
				}
				Materials.Add(ShapeBarrier.GetRenderMaterial());
			}
		};

		auto CollectFromBodies = [&CollectFromShapes](const TArray<FRigidBodyBarrier>& BodyBarriers)
		{
			for (const FRigidBodyBarrier& BodyBarrier : BodyBarriers)
			{
				CollectFromShapes(BodyBarrier.GetShapes());
			}
		};

		CollectFromShapes(SimulationObjects.GetBoxShapes());
		CollectFromShapes(SimulationObjects.GetCapsuleShapes());
		CollectFromShapes(SimulationObjects.GetCylinderShapes());
		CollectFromShapes(SimulationObjects.GetSphereShapes());
		CollectFromShapes(SimulationObjects.GetTrimeshShapes());
		CollectFromBodies(SimulationObjects.GetRigidBodies());

		return Materials;
	}

	void AddOrUpdateRenderMaterials(
		const FSimulationObjectCollection& SimulationObjects, SCSNodeCollection& SCSNodes,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		TArray<FAGX_RenderMaterial> Materials = CollectRenderMaterials(SimulationObjects);
		for (FAGX_RenderMaterial& Material : Materials)
		{
			const FGuid Guid = Material.Guid;
			UMaterialInstanceConstant* Asset = Helper.GetRenderMaterial(Guid);
			if (Asset != nullptr)
			{
				Helper.UpdateAndSaveRenderMaterialAsset(Material, *Asset);
			}
			else
			{
				/// @todo Create a new Render Material here.
				Helper.InstantiateRenderMaterial(Material);
			}
		}
	}

	USCS_Node* GetOrCreateContactMaterialRegistrarNode(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes)
	{
		if (SCSNodes.ContactMaterialRegistrarComponent != nullptr)
		{
			return SCSNodes.ContactMaterialRegistrarComponent;
		}

		USCS_Node* NewNode = BaseBP.SimpleConstructionScript->CreateNode(
			UAGX_ContactMaterialRegistrarComponent::StaticClass(),
			FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
		BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(NewNode);
		SCSNodes.ContactMaterialRegistrarComponent = NewNode;

		return NewNode;
	}

	void ReParentNode(UBlueprint& BaseBP, USCS_Node& Node, USCS_Node& NewParent)
	{
		USCS_Node* OldParent = BaseBP.SimpleConstructionScript->FindParentNode(&Node);
		if (OldParent != nullptr)
		{
			OldParent->RemoveChildNode(&Node);
		}

		NewParent.AddChildNode(&Node);
	}

	void AddOrUpdateContactMaterials(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper)
	{
		USCS_Node* CMRegistrarNode = GetOrCreateContactMaterialRegistrarNode(BaseBP, SCSNodes);
		const FString CMRName = FAGX_ImportUtilities::GetContactMaterialRegistrarDefaultName();
		CMRegistrarNode->SetVariableName(*CMRName);
		auto CMRegistrar =
			Cast<UAGX_ContactMaterialRegistrarComponent>(CMRegistrarNode->ComponentTemplate);

		// For Contact Materials in SimulationObjects that do have an asset, update that asset
		// to match the new SimulationObjects Contact Material.
		//
		// For Contact Materials in SimulationObjects that do not have an asset, create new Contact
		// Material assets.
		const FString ContactMaterialDirPath =
			GetImportDirPath(Helper, FAGX_ImportUtilities::GetImportContactMaterialDirectoryName());
		const TMap<FGuid, UAGX_ContactMaterial*> ExistingContactMaterialsMap =
			FindAGXAssetComponents<UAGX_ContactMaterial>(ContactMaterialDirPath);
		for (const auto& Barrier : SimulationObjects.GetContactMaterials())
		{
			UAGX_ContactMaterial* Asset = ExistingContactMaterialsMap.FindRef(Barrier.GetGuid());
			if (Asset != nullptr)
			{
				Helper.UpdateAndSaveContactMaterialAsset(Barrier, *Asset, *CMRegistrar);
			}
			else
			{
				Helper.InstantiateContactMaterial(Barrier, *CMRegistrar);
			}
		}
	}

	template <typename TBarrier, typename TComponent>
	USCS_Node* AddOrUpdateShape(
		const TBarrier& Barrier, UBlueprint& BaseBP, const FAGX_SynchronizeModelSettings& Settings,
		TMap<FGuid, USCS_Node*>& ExistingShapes, FAGX_SimObjectsImporterHelper& Helper,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		USCS_Node* OverrideAttachParent = nullptr)
	{
		USCS_Node* AttachParent = OverrideAttachParent != nullptr
									  ? OverrideAttachParent
									  : BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();

		const FGuid Guid = Barrier.GetShapeGuid();
		USCS_Node* Node = nullptr;
		if (ExistingShapes.Contains(Guid))
		{
			Node = ExistingShapes[Guid];
			ReParentNode(BaseBP, *Node, *AttachParent);
		}
		else
		{
			Node = BaseBP.SimpleConstructionScript->CreateNode(
				TComponent::StaticClass(), FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			AttachParent->AddChildNode(Node);
			ExistingShapes.Add(Guid, Node);
		}

		Helper.UpdateComponent(
			Barrier, *Cast<TComponent>(Node->ComponentTemplate), MSTsOnDisk,
			Settings.bForceOverwriteProperties, Settings.bForceReassignRenderMaterials);
		return Node;
	}

	void AddOrUpdateRenderData(
		const FShapeBarrier& ShapeBarrier, USCS_Node& AttachParent, UBlueprint& BaseBP,
		SCSNodeCollection& SCSNodes, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		if (!ShapeBarrier.HasRenderData())
			return;

		const FRenderDataBarrier RenderDataBarrier = ShapeBarrier.GetRenderData();
		if (!RenderDataBarrier.HasMesh())
			return;

		const FGuid RenderDataGuid = RenderDataBarrier.GetGuid();
		USCS_Node* RenderDataNode = nullptr;
		if (SCSNodes.RenderStaticMeshComponents.Contains(RenderDataGuid))
		{
			RenderDataNode = SCSNodes.RenderStaticMeshComponents[RenderDataGuid];

			// Render Data may need to re-parent in the case that this model was imported with a
			// different import setting for "Ignore Disabled Trimeshes" than the original import.
			ReParentNode(BaseBP, *RenderDataNode, AttachParent);
		}
		else
		{
			RenderDataNode = BaseBP.SimpleConstructionScript->CreateNode(
				UStaticMeshComponent::StaticClass(),
				FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			AttachParent.AddChildNode(RenderDataNode);
			SCSNodes.RenderStaticMeshComponents.Add(RenderDataGuid, RenderDataNode);
		}

		Helper.UpdateRenderDataComponent(
			ShapeBarrier, RenderDataBarrier,
			*Cast<UStaticMeshComponent>(RenderDataNode->ComponentTemplate),
			Settings.bForceOverwriteProperties, Settings.bForceReassignRenderMaterials);
	}

	void AddOrUpdateRigidBodiesAndOwnedShapes(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ExistingMSTAssets)
	{
		for (const auto& RbBarrier : SimulationObjects.GetRigidBodies())
		{
			const FGuid Guid = RbBarrier.GetGuid();
			USCS_Node* RigidBodyNode = nullptr;
			if (SCSNodes.RigidBodies.Contains(Guid))
			{
				RigidBodyNode = SCSNodes.RigidBodies[Guid];
			}
			else
			{
				RigidBodyNode = BaseBP.SimpleConstructionScript->CreateNode(
					UAGX_RigidBodyComponent::StaticClass(),
					FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
				BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(
					RigidBodyNode);
				SCSNodes.RigidBodies.Add(Guid, RigidBodyNode);
			}

			Helper.UpdateRigidBodyComponent(
				RbBarrier, *Cast<UAGX_RigidBodyComponent>(RigidBodyNode->ComponentTemplate),
				ExistingMSTAssets, Settings.bForceOverwriteProperties);

			// Add or update all shapes in the current Rigid Body.
			for (const auto& ShapeBarrier : RbBarrier.GetSphereShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_SphereShapeComponent>(
						ShapeBarrier, BaseBP, Settings, SCSNodes.SphereShapes, Helper,
						ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetBoxShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_BoxShapeComponent>(
						ShapeBarrier, BaseBP, Settings, SCSNodes.BoxShapes, Helper,
						ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetCylinderShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_CylinderShapeComponent>(
						ShapeBarrier, BaseBP, Settings, SCSNodes.CylinderShapes, Helper,
						ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetCapsuleShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_CapsuleShapeComponent>(
						ShapeBarrier, BaseBP, Settings, SCSNodes.CapsuleShapes, Helper,
						ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetTrimeshShapes())
			{
				USCS_Node* RenderDataParent = nullptr;
				if (!ShapeBarrier.GetEnableCollisions() && Settings.bIgnoreDisabledTrimeshes)
				{
					RenderDataParent = RigidBodyNode;
				}
				else
				{
					USCS_Node* ShapeNode =
						AddOrUpdateShape<decltype(ShapeBarrier), UAGX_TrimeshShapeComponent>(
							ShapeBarrier, BaseBP, Settings, SCSNodes.TrimeshShapes, Helper,
							ExistingMSTAssets, RigidBodyNode);

					// The ShapeNode Trimesh returned from AddOrUpdateShape above may or may not be
					// a new Trimesh. If it is new, we need to create a new collision mesh for it.
					USCS_Node* CollisionMesh = nullptr;
					if (ShapeNode->GetChildNodes().Num() == 0)
					{
						CollisionMesh = BaseBP.SimpleConstructionScript->CreateNode(
							UStaticMeshComponent::StaticClass(),
							FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
						ShapeNode->AddChildNode(CollisionMesh);
						SCSNodes.CollisionStaticMeshComponents.Add(
							ShapeBarrier.GetShapeGuid(), CollisionMesh);
					}
					else
					{
						CollisionMesh = ShapeNode->GetChildNodes()[0];
					}

					Helper.UpdateTrimeshCollisionMeshComponent(
						ShapeBarrier, *Cast<UStaticMeshComponent>(CollisionMesh->ComponentTemplate),
						Settings.bForceOverwriteProperties, Settings.bForceReassignRenderMaterials);
					RenderDataParent = CollisionMesh;
				}

				AddOrUpdateRenderData(
					ShapeBarrier, *RenderDataParent, BaseBP, SCSNodes, Helper, Settings);
			}
		}
	}

	void AddOrUpdateBodilessShapes(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ExistingMSTAssets)
	{
		for (const auto& Barrier : SimulationObjects.GetSphereShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_SphereShapeComponent>(
				Barrier, BaseBP, Settings, SCSNodes.SphereShapes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
		}

		for (const auto& Barrier : SimulationObjects.GetBoxShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_BoxShapeComponent>(
				Barrier, BaseBP, Settings, SCSNodes.BoxShapes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
		}

		for (const auto& Barrier : SimulationObjects.GetCylinderShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_CylinderShapeComponent>(
				Barrier, BaseBP, Settings, SCSNodes.CylinderShapes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
		}

		for (const auto& Barrier : SimulationObjects.GetCapsuleShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_CapsuleShapeComponent>(
				Barrier, BaseBP, Settings, SCSNodes.CapsuleShapes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper, Settings);
		}

		for (const auto& Barrier : SimulationObjects.GetTrimeshShapes())
		{
			USCS_Node* RenderDataParent = nullptr;
			if (!Barrier.GetEnableCollisions() && Settings.bIgnoreDisabledTrimeshes)
			{
				RenderDataParent = BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();
			}
			else
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(Barrier), UAGX_TrimeshShapeComponent>(
						Barrier, BaseBP, Settings, SCSNodes.TrimeshShapes, Helper,
						ExistingMSTAssets);

				// The ShapeNode Trimesh returned from AddOrUpdateShape above may or may not be a
				// new Trimesh. If it is new, we need to create a new collision mesh for it.
				USCS_Node* CollisionMesh = nullptr;
				if (ShapeNode->GetChildNodes().Num() == 0)
				{
					CollisionMesh = BaseBP.SimpleConstructionScript->CreateNode(
						UStaticMeshComponent::StaticClass(),
						FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
					ShapeNode->AddChildNode(CollisionMesh);
					SCSNodes.CollisionStaticMeshComponents.Add(
						Barrier.GetShapeGuid(), CollisionMesh);
				}
				else
				{
					CollisionMesh = ShapeNode->GetChildNodes()[0];
				}

				Helper.UpdateTrimeshCollisionMeshComponent(
					Barrier, *Cast<UStaticMeshComponent>(CollisionMesh->ComponentTemplate),
					Settings.bForceOverwriteProperties, Settings.bForceReassignRenderMaterials);

				RenderDataParent = CollisionMesh;
			}

			AddOrUpdateRenderData(Barrier, *RenderDataParent, BaseBP, SCSNodes, Helper, Settings);
		}
	}

	void AddOrUpdateConstraints(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ExistingMSTAssets)
	{
		USCS_Node* RootNode = BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();

		// Returns true if a new Constraint was created, false otherwise.
		auto AddOrUpdateConstraint = [&](const auto& NewConstraints,
										 const TMap<FGuid, USCS_Node*>& BPConstraints,
										 UClass* ConstraintClass) -> TMap<FGuid, USCS_Node*>
		{
			TMap<FGuid, USCS_Node*> CreatedConstraints;
			for (const auto& Barrier : NewConstraints)
			{
				const FGuid ConstraintGuid = Barrier.GetGuid();
				USCS_Node* Constraint = nullptr;
				if (BPConstraints.Contains(ConstraintGuid))
				{
					Constraint = BPConstraints[ConstraintGuid];
				}
				else
				{
					Constraint = BaseBP.SimpleConstructionScript->CreateNode(
						ConstraintClass, FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
					RootNode->AddChildNode(Constraint);
					CreatedConstraints.Add(ConstraintGuid, Constraint);
				}
				Helper.UpdateConstraintComponent(
					Barrier, *Cast<UAGX_ConstraintComponent>(Constraint->ComponentTemplate),
					ExistingMSTAssets, Settings.bForceOverwriteProperties);
			}
			return CreatedConstraints;
		};

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetHingeConstraints(), SCSNodes.HingeConstraints,
				 UAGX_HingeConstraintComponent::StaticClass()))
		{
			SCSNodes.HingeConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetPrismaticConstraints(), SCSNodes.PrismaticConstraints,
				 UAGX_PrismaticConstraintComponent::StaticClass()))
		{
			SCSNodes.PrismaticConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetBallConstraints(), SCSNodes.BallConstraints,
				 UAGX_BallConstraintComponent::StaticClass()))
		{
			SCSNodes.BallConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetCylindricalConstraints(), SCSNodes.CylindricalConstraints,
				 UAGX_CylindricalConstraintComponent::StaticClass()))
		{
			SCSNodes.CylindricalConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetDistanceConstraints(), SCSNodes.DistanceConstraints,
				 UAGX_DistanceConstraintComponent::StaticClass()))
		{
			SCSNodes.DistanceConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}

		for (const auto& CreatedConstraintsTuple : AddOrUpdateConstraint(
				 SimulationObjects.GetLockConstraints(), SCSNodes.LockConstraints,
				 UAGX_LockConstraintComponent::StaticClass()))
		{
			SCSNodes.LockConstraints.Add(
				CreatedConstraintsTuple.Key, CreatedConstraintsTuple.Value);
		}
	}

	void AddOrUpdateTwoBodyTires(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		for (const auto& Barrier : SimulationObjects.GetTwoBodyTires())
		{
			const FGuid Guid = Barrier.GetGuid();
			USCS_Node* TireNode = nullptr;
			if (SCSNodes.TwoBodyTires.Contains(Guid))
			{
				TireNode = SCSNodes.TwoBodyTires[Guid];
			}
			else
			{
				TireNode = BaseBP.SimpleConstructionScript->CreateNode(
					UAGX_TwoBodyTireComponent::StaticClass(),
					FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
				BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(TireNode);
				SCSNodes.TwoBodyTires.Add(Guid, TireNode);
			}

			Helper.UpdateTwoBodyTire(
				Barrier, *Cast<UAGX_TwoBodyTireComponent>(TireNode->ComponentTemplate),
				Settings.bForceOverwriteProperties);
		}
	}

	void AddOrUpdateCollisionGroupDisabler(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper)
	{
		USCS_Node* DisablerNode = nullptr;
		if (SCSNodes.CollisionGroupDisablerComponent != nullptr)
		{
			DisablerNode = SCSNodes.CollisionGroupDisablerComponent;
		}
		else
		{
			if (SimulationObjects.GetDisabledCollisionGroups().Num() == 0)
			{
				// In the case that no CollisionGroupDisablerComponent existed before the model
				// synchronization, and there are no disabled Collision Groups in the model being
				// imported, there is no need to create a CollisionGroupDisablerComponent. We
				// are done.
				return;
			}

			DisablerNode = BaseBP.SimpleConstructionScript->CreateNode(
				UAGX_CollisionGroupDisablerComponent::StaticClass(),
				FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(DisablerNode);
			SCSNodes.CollisionGroupDisablerComponent = DisablerNode;
		}

		Helper.UpdateCollisionGroupDisabler(
			SimulationObjects.GetDisabledCollisionGroups(),
			*Cast<UAGX_CollisionGroupDisablerComponent>(DisablerNode->ComponentTemplate));
	}

	void AddOrUpdateObserverFrames(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		for (const auto& ObserverFrame : SimulationObjects.GetObserverFrames())
		{
			const FGuid Guid = ObserverFrame.ObserverGuid;
			USCS_Node* ObserverNode = nullptr;
			USCS_Node* AttachParent = nullptr;
			if (SCSNodes.RigidBodies.Contains(ObserverFrame.BodyGuid))
			{
				AttachParent = SCSNodes.RigidBodies[ObserverFrame.BodyGuid];
			}
			else
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Could not find Rigid Body with Guid '%s' that should act as the "
						 "attach parent for Observer Frame '%s'. The Observer Frame will be "
						 "attached to the root Component instead."),
					*ObserverFrame.BodyGuid.ToString(), *ObserverFrame.Name);
				AttachParent = BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();
			}

			if (SCSNodes.ObserverFrames.Contains(Guid))
			{
				ObserverNode = SCSNodes.ObserverFrames[Guid];
				ReParentNode(BaseBP, *ObserverNode, *AttachParent);
			}
			else
			{
				ObserverNode = BaseBP.SimpleConstructionScript->CreateNode(
					UAGX_ObserverFrameComponent::StaticClass(),
					FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
				AttachParent->AddChildNode(ObserverNode);
				SCSNodes.ObserverFrames.Add(ObserverFrame.ObserverGuid, ObserverNode);
			}

			Helper.UpdateObserverFrameComponent(
				ObserverFrame.Name, ObserverFrame.ObserverGuid, ObserverFrame.Transform,
				*Cast<UAGX_ObserverFrameComponent>(ObserverNode->ComponentTemplate),
				Settings.bForceOverwriteProperties);
		}
	}

	void AddOrUpdateModelSourceComponent(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes, FAGX_SimObjectsImporterHelper& Helper)
	{
		USCS_Node* ModelSourceComponent = nullptr;
		if (SCSNodes.ModelSourceComponent == nullptr)
		{
			ModelSourceComponent = BaseBP.SimpleConstructionScript->CreateNode(
				UAGX_ModelSourceComponent::StaticClass(),
				FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(
				ModelSourceComponent);
			SCSNodes.ModelSourceComponent = ModelSourceComponent;
		}
		else
		{
			ModelSourceComponent = SCSNodes.ModelSourceComponent;
		}

		/// @todo Helper.UpdateModelSourceComponent will set
		/// ModelSourceComponent.UnrealMaterialToImportGuid to the contents of
		/// Helper.RestoredRenderMaterials. Has RestoredRenderMaterials been updated at this point,
		/// i.e., has Helper.RestoredRenderMaterials been initialized with the contents of
		/// ModelSourceComponent.UnrealMaterialToImportGuid?
		///
		/// If not synchronizing a model a second time would not delete Render Material assets
		/// for AGX Dynamics render materials that no longer exists in the AGX Dynamics archive
		/// because we wouldn't know that import GUID of those Render Material assets since they
		/// are stored in ModelSourceComponent.UnrealMaterialToImportGuid and not the asset itself
		/// since we have not yet found a way to extend engine assets with additional properties.
		Helper.UpdateModelSourceComponent(
			*Cast<UAGX_ModelSourceComponent>(ModelSourceComponent->ComponentTemplate));
	}

	FString GetModelDirectoryFromAsset(UObject* Asset)
	{
		if (Asset == nullptr)
		{
			return "";
		}

		TArray<FString> Splits;
		Asset->GetPathName().ParseIntoArray(Splits, TEXT("/"));
		for (int i = 0; i < Splits.Num(); i++)
		{
			if (Splits[i].Equals(FAGX_ImportUtilities::GetImportRootDirectoryName()))
			{
				return i + 1 < Splits.Num() ? Splits[i + 1] : "";
			}
		}
		return "";
	}

	// The passed SCSNodes will be kept up to date, i.e. elements added to BaseBP will have their
	// corresponding SCS Node removed from SCSNodes as well.
	void AddOrUpdateAll(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects,
		const FAGX_SynchronizeModelSettings& Settings, FAGX_SimObjectsImporterHelper& Helper)
	{
		FScopedSlowTask ImportTask(105.f, LOCTEXT("AddOrUpdateAll", "Adding new data"), true);
		ImportTask.MakeDialog();

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Adding data"));

		// We collect all existing merge split thresholds assets here once, and pass it down to the
		// relevant AddOrUpdate functions below, instead of looking it up several times from within
		// those functions.
		const FString MSTDirPath = GetImportDirPath(
			Helper, FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName());
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*> ExistingMSTAssets =
			FindAGXAssetComponents<UAGX_MergeSplitThresholdsBase>(MSTDirPath);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Synchronizing Shape Materials"));
		AddOrUpdateShapeMaterials(BaseBP, SimulationObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Synchronizing Contact Materials"));
		AddOrUpdateContactMaterials(BaseBP, SCSNodes, SimulationObjects, Helper);

		ImportTask.EnterProgressFrame(5.0f, FText::FromString("Synchronizing Render Materials"));
		AddOrUpdateRenderMaterials(SimulationObjects, SCSNodes, Helper);

		ImportTask.EnterProgressFrame(
			5.f, FText::FromString("Synchronizing Rigid Bodies and Shapes"));
		AddOrUpdateRigidBodiesAndOwnedShapes(
			BaseBP, SCSNodes, SimulationObjects, Helper, Settings, ExistingMSTAssets);

		ImportTask.EnterProgressFrame(15.f, FText::FromString("Synchronizing Bodiless Shapes"));
		AddOrUpdateBodilessShapes(
			BaseBP, SCSNodes, SimulationObjects, Helper, Settings, ExistingMSTAssets);

		ImportTask.EnterProgressFrame(15.f, FText::FromString("Synchronizing Constraints"));
		AddOrUpdateConstraints(
			BaseBP, SCSNodes, SimulationObjects, Helper, Settings, ExistingMSTAssets);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Synchronizing Tire Models"));
		AddOrUpdateTwoBodyTires(BaseBP, SCSNodes, SimulationObjects, Helper, Settings);

		ImportTask.EnterProgressFrame(
			5.f, FText::FromString("Synchronizing Collision Groups Disabler"));
		AddOrUpdateCollisionGroupDisabler(BaseBP, SCSNodes, SimulationObjects, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Synchronizing Observer Frames"));
		AddOrUpdateObserverFrames(BaseBP, SCSNodes, SimulationObjects, Helper, Settings);

		ImportTask.EnterProgressFrame(
			5.f, FText::FromString("Synchronizing Model Source Component"));
		AddOrUpdateModelSourceComponent(BaseBP, SCSNodes, Helper);

		ImportTask.EnterProgressFrame(30.f, FText::FromString("Finalizing Synchronization"));
		Helper.FinalizeImport();
	}

	void RemoveDeletedRigidBodies(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects)
	{
		const TArray<FGuid> BarrierGuids = GetGuidsFromBarriers(SimulationObjects.GetRigidBodies());
		for (auto It = SCSNodes.RigidBodies.CreateIterator(); It; ++It)
		{
			if (!BarrierGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
		}
	}

	void RemoveDeletedShapes(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FAGX_SynchronizeModelSettings& Settings, const FShapeGuidsCollection& NewShapeGuids)
	{
		auto RemoveUnmatched =
			[&BaseBP](const TMap<FGuid, bool>& NewGuids, TMap<FGuid, USCS_Node*>& ExistingGuids)
		{
			for (auto It = ExistingGuids.CreateIterator(); It; ++It)
			{
				if (!NewGuids.Contains(It->Key))
				{
					BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
					It.RemoveCurrent();
				}
			}
		};

		// Primitive Shapes.
		RemoveUnmatched(NewShapeGuids.SphereShapeGuids, SCSNodes.SphereShapes);
		RemoveUnmatched(NewShapeGuids.BoxShapeGuids, SCSNodes.BoxShapes);
		RemoveUnmatched(NewShapeGuids.CylinderShapeGuids, SCSNodes.CylinderShapes);
		RemoveUnmatched(NewShapeGuids.CapsuleShapeGuids, SCSNodes.CapsuleShapes);

		// Trimesh Shapes, needing some special handing due to import settings.
		for (auto It = SCSNodes.TrimeshShapes.CreateIterator(); It; ++It)
		{
			if (!NewShapeGuids.TrimeshShapeGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
			else
			{
				// If we synchronize with the "Ignore disabled Trimeshes" import setting and this
				// Trimesh has collision disabled, it should be removed.
				const bool CollisionEnabled = NewShapeGuids.TrimeshShapeGuids[It->Key];
				if (!CollisionEnabled && Settings.bIgnoreDisabledTrimeshes)
				{
					BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
					It.RemoveCurrent();
				}
			}
		}
	}

	void RemoveDeletedStaticMeshComponents(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes, const FShapeGuidsCollection& NewShapeGuids,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		// Remove deleted Render Data.
		for (auto It = SCSNodes.RenderStaticMeshComponents.CreateIterator(); It; ++It)
		{
			if (!NewShapeGuids.RenderDataGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
		}

		// Remove deleted collision meshes (only relevant for Trimesh Shapes).
		for (auto It = SCSNodes.CollisionStaticMeshComponents.CreateIterator(); It; ++It)
		{
			if (!NewShapeGuids.TrimeshShapeGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
			else
			{
				USCS_Node* CollisionStaticMeshComponentNode = It->Value;

				// A collision Static Mesh Component should be removed even if it's owning Trimesh
				// exists in SimulationObjects if the following conditions are true:
				// 1. The import setting 'bIgnoreDisabledTrimeshes' is used during this
				// model synchronization.
				// 2. The AGX Trimesh that is being imported has collision disabled.
				if (!Settings.bIgnoreDisabledTrimeshes)
				{
					continue;
				}

				if (NewShapeGuids.TrimeshShapeGuids[It->Key])
				{
					continue;
				}

				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(
					CollisionStaticMeshComponentNode);
				It.RemoveCurrent();
			}
		}

		// @todo We should remove the Static Mesh Assets pointed to by the removed Static Mesh
		// Components above. Do this when we figure out how to safely remove assets and resolve any
		// references to it.
	}

	void RemoveDeletedConstraints(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects)
	{
		auto RemoveDeletedConstraint =
			[&](const auto& NewConstraints, TMap<FGuid, USCS_Node*>& OldConstraints)
		{
			const TArray<FGuid> NewConstraintGuids = GetGuidsFromBarriers(NewConstraints);
			for (auto It = OldConstraints.CreateIterator(); It; ++It)
			{
				if (!NewConstraintGuids.Contains(It->Key))
				{
					BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
					It.RemoveCurrent();
				}
			}
		};

		RemoveDeletedConstraint(SimulationObjects.GetHingeConstraints(), SCSNodes.HingeConstraints);
		RemoveDeletedConstraint(
			SimulationObjects.GetPrismaticConstraints(), SCSNodes.PrismaticConstraints);
		RemoveDeletedConstraint(SimulationObjects.GetBallConstraints(), SCSNodes.BallConstraints);
		RemoveDeletedConstraint(
			SimulationObjects.GetCylindricalConstraints(), SCSNodes.CylindricalConstraints);
		RemoveDeletedConstraint(
			SimulationObjects.GetDistanceConstraints(), SCSNodes.DistanceConstraints);
		RemoveDeletedConstraint(SimulationObjects.GetLockConstraints(), SCSNodes.LockConstraints);
	}

	void RemoveDeletedTireModels(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects)
	{
		const TArray<FGuid> BarrierGuids =
			GetGuidsFromBarriers(SimulationObjects.GetTwoBodyTires());
		for (auto It = SCSNodes.TwoBodyTires.CreateIterator(); It; ++It)
		{
			if (!BarrierGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
		}
	}

	void RemoveDeletedObserverFrames(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects)
	{
		TArray<FGuid> ObserverGuids;
		ObserverGuids.Reserve(SimulationObjects.GetObserverFrames().Num());
		for (const auto& ObserverFrame : SimulationObjects.GetObserverFrames())
		{
			ObserverGuids.Add(ObserverFrame.ObserverGuid);
		}

		for (auto It = SCSNodes.ObserverFrames.CreateIterator(); It; ++It)
		{
			if (!ObserverGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
		}
	}

	/**
	 * Delete assets that are no longer used.
	 *
	 * We find such assets by enumerating the SCS collection and comparing each with the simulation
	 * object collection. Any asset that is found in the SCS collection but not found in the
	 * simulation object collection is deleted.
	 *
	 * The above is a bad idea. If the user has edited the Blueprint to point to some other asset
	 * unrelated to the imported model then that asset won't be found among the simulation objects
	 * during model synchronization, since it was never part of the model, and thus deleted. We
	 * should not delete assets we didn't import. They may be used for other purposes. It is better
	 * to enumerate the assets in the import folder on drive, those are precisely the assets that
	 * were imported with the model. The user should not add assets there themselves.
	 *
	 * Comparison is done on GUID/UUIDs.
	 *
	 * Assets of build-in Unreal Engine types, such as Materials, don't have an Import GUID. Those
	 * GUID's are instead stored in a look-up table in an UAGX_ModelSourceComponent. This function
	 * should be called before the AGX Dynamics archive is applied to the Blueprint since here we
	 * need to see the old list of assets, not the new ones.
	 *
	 * Mesh assets are currently not comparable, i.e. they can get new triangle data but keep the
	 * same GUID and we cannot compare triangle by triangle because Unreal Engine modifies them
	 * when building the Static Mesh asset, so they are always deleted and recreated for now.
	 *
	 * General steps:
	 * - Find all objects in the simulation objects collection.
	 *   These are the ones whose corresponding assets we want to keep.
	 * - Find all assets on drive.
	 *   These are assets that were created during prior imports or synchronizations.
	 * - Compare the simulation objects and the assets.
	 *   All assets that don't have a corresponding simulation object is added to the delete list.
	 * - Do any extra cleanup required for the asset type.
	 *   Deleting the asset will set references to it to None / nullptr.
	 *   In some cases we want to avoid that, for example the Contact Materials list in Contact
	 *   Material Registrar.
	 *   We handle that by finding such nodes in the SCSNodes list and purging about-to-become-None
	 *   elements in the node's template component's collection property.
	 *
	 * The rest of the steps happens elsewhere, not in DeleteRemovedAssets.
	 * - Populate Restored... in AGX_SimObjectsImporterHelper.
	 *   This is done implicitly by the AddOrUpdate... functions in this file.
	 *   Those functions iterate through the simulation objects and either create a new asset or
	 *   updates the existing one based on whether or not an assets exists on drive.
	 *   Both operations are done though the helper, which will add the new or updated asset to
	 *   Restored... as part of the Instantiate... or UpdateAndSave... functions.
	 *   There can be no additional assets on drive since this function removed any asset that
	 *   doesn't have a corresponding simulation object.
	 */
	void DeleteRemovedAssets(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		TArray<UObject*> AssetsToDelete;

		// Delete removed Contact Materials.
		UE_LOG(LogAGX, Warning, TEXT("Contact Materials:"));
		if (SCSNodes.ContactMaterialRegistrarComponent != nullptr)
		{
			if (auto Registrar = Cast<UAGX_ContactMaterialRegistrarComponent>(
					SCSNodes.ContactMaterialRegistrarComponent->ComponentTemplate))
			{
				const TArray<FContactMaterialBarrier>& Barriers =
					SimulationObjects.GetContactMaterials();

				// Track which assets are added to the AssetsToDelete list. Deleting assets will
				// set any references to that asset to None / nullptr which is correct in some cases
				// but for the Contact Material Registrar we remove the elements completely to avoid
				// leaving nullptr entries in the array.
				TSet<UAGX_ContactMaterial*> Removed;

				// Queue for deletion any Contact Material that we find in the Blueprint that no
				// longer exists in among the Simulation Objects.
				for (UAGX_ContactMaterial* Asset : Registrar->ContactMaterials)
				{
					const FGuid Guid = Asset->ImportGuid;
					UE_LOG(LogAGX, Warning, TEXT("  Checking asset GUID %s..."), *Guid.ToString());
					auto Predicate = [&Guid](const FContactMaterialBarrier& Barrier)
					{
						UE_LOG(
							LogAGX, Warning, TEXT("    ...against imported GUID %s: %s"),
							*Barrier.GetGuid().ToString(),
							(Guid == Barrier.GetGuid() ? TEXT("Match") : TEXT("No")));
						return Guid == Barrier.GetGuid();
					};
					if (!Barriers.ContainsByPredicate(Predicate))
					{
						UE_LOG(
							LogAGX, Warning,
							TEXT("Did not find a match, adding asset to delete list."));
						AssetsToDelete.AddUnique(Asset);
						Removed.Add(Asset);
					}
				}

				// Purge references to the soon-to-be-removed assets from the Contact Material
				// Registrar and all its archetype instances.
				auto Predicate = [Removed](const UAGX_ContactMaterial* Element)
				{ return Removed.Contains(Element); };
				Registrar->ContactMaterials.RemoveAll(Predicate);
				for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Registrar))
				{
					Instance->ContactMaterials.RemoveAll(Predicate);
				}
			}
		}

		// Delete removed Constraint Merge Split Thresholds.
		{
			UE_LOG(LogAGX, Warning, TEXT("\nConstraint Merge Split Thresholds:"));
			// We want to find all Merge Split Thresholds in SCSNodes that does not exist in
			// SimulationObjects. There is no pre-collected list of them in either of the two
			// collections so we create it here. We can find Merge Split Thresholds on Constraints,
			// Shapes, and Wires.

			// Collect Merge Split Thresholds from Blueprint constraint nodes.
			// These are the assets that may need to be deleted.
			TArray<UAGX_ConstraintMergeSplitThresholds*> Assets;
			UE_LOG(LogAGX, Warning, TEXT("Collecting Blueprint thresholds:"));
			auto CollectFromBlueprint = [&Assets](TMap<FGuid, USCS_Node*>& Constraints)
			{
				for (const auto It : Constraints)
				{
					const UAGX_ConstraintComponent* const Constraint =
						Cast<UAGX_ConstraintComponent>(It.Value->ComponentTemplate);
					if (Constraint == nullptr)
					{
						// Not all SCS nodes have a Component Template. Not sure if a Constraint
						// node can have a Component Template that isn't a Constraint Component.
						continue;
					}

					UAGX_ConstraintMergeSplitThresholds* Asset =
						Constraint->MergeSplitProperties.Thresholds;
					if (Asset == nullptr)
					{
						// Note all Constraints have a Merge Split Thresholds.
						continue;
					}

					Assets.AddUnique(Asset);
					UE_LOG(LogAGX, Warning, TEXT("  %s"), *Asset->ImportGuid.ToString());
				}
			};

			/// @todo Consider storing a list of all Constraints in SCSNodes so we don't need to
			/// operate on so many collections here.
			///
			/// @todo Why traverse the SCS nodes instead of asking FindAGXAssetComponents for a list
			/// of all assets in the import directory? This approach will do bad things if the
			/// Blueprint has been changed to point to assets outside of the model's import
			/// directory. We should not delete such assets.
			CollectFromBlueprint(SCSNodes.BallConstraints);
			CollectFromBlueprint(SCSNodes.CylindricalConstraints);
			CollectFromBlueprint(SCSNodes.DistanceConstraints);
			CollectFromBlueprint(SCSNodes.HingeConstraints);
			CollectFromBlueprint(SCSNodes.LockConstraints);
			CollectFromBlueprint(SCSNodes.PrismaticConstraints);

			// Collect Merge Split Thresholds from the simulation objects.
			// Any Threshold we find should not have its asset deleted.
			UE_LOG(LogAGX, Warning, TEXT("Collecting simulation thresholds:"));
			TSet<FGuid> NotRemoved;
			auto CollectFromSimulation = [&NotRemoved](const auto& ConstraintBarriers)
			{
				for (auto& ConstraintBarrier : ConstraintBarriers)
				{
					const FConstraintMergeSplitThresholdsBarrier ThresholdsBarrier =
						FConstraintMergeSplitThresholdsBarrier::CreateFrom(ConstraintBarrier);
					if (!ThresholdsBarrier.HasNative())
					{
						// Not all Constraints have a Merge Split Thresholds.
						return;
					}
					const FGuid Guid = ThresholdsBarrier.GetGuid();
					NotRemoved.Add(Guid);
					UE_LOG(LogAGX, Warning, TEXT("  %s"), *Guid.ToString());
				}
			};

			CollectFromSimulation(SimulationObjects.GetBallConstraints());
			CollectFromSimulation(SimulationObjects.GetCylindricalConstraints());
			CollectFromSimulation(SimulationObjects.GetDistanceConstraints());
			CollectFromSimulation(SimulationObjects.GetHingeConstraints());
			CollectFromSimulation(SimulationObjects.GetLockConstraints());
			CollectFromSimulation(SimulationObjects.GetPrismaticConstraints());

			// Mark any asset not found among the simulation objects for deletion.
			UE_LOG(LogAGX, Warning, TEXT("Checking for matches:"));
			for (UAGX_ConstraintMergeSplitThresholds* Asset : Assets)
			{
				const FGuid AssetGui = Asset->ImportGuid;
				UE_LOG(LogAGX, Warning, TEXT("  Checking asset %s."), *AssetGui.ToString())
				if (!NotRemoved.Contains(AssetGui))
				{
					UE_LOG(LogAGX, Warning, TEXT("  Gone from simulation, deleting asset."))
					AssetsToDelete.Add(Asset);
				}
			}
		}

		// Delete all render and collision meshes.
		//
		// We currently can't reuse the old assets because we have no way of knowing if they have
		// been changed or not since we, unfortunately, store these meshes as Unreal Engine Static
		// Mesh assets which Unreal Engine changes during import. For the render mesh we may not
		// have a choice since they are for rendering, but the collision mesh should perhaps be
		// a custom asset whose storage is byte-compatible with AGX Dynamics trimesh.
		{
			const FString RenderMeshDirPath =
				GetImportDirPath(Helper, FAGX_ImportUtilities::GetImportRenderMeshDirectoryName());
			const FString CollisionMeshDirPath =
				GetImportDirPath(Helper, FAGX_ImportUtilities::GetImportStaticMeshDirectoryName());

			TArray<UStaticMesh*> RenderMeshes =
				FAGX_EditorUtilities::FindAssets<UStaticMesh>(RenderMeshDirPath);
			TArray<UStaticMesh*> CollisionMeshes =
				FAGX_EditorUtilities::FindAssets<UStaticMesh>(CollisionMeshDirPath);

			UE_LOG(LogAGX, Warning, TEXT("Render meshes: "), *RenderMeshDirPath);
			for (UStaticMesh* Mesh : RenderMeshes)
			{
				UE_LOG(LogAGX, Warning, TEXT("  %s"), *Mesh->GetPathName());
				AssetsToDelete.AddUnique(Mesh);
			}
			UE_LOG(LogAGX, Warning, TEXT("Collision meshes: "), *CollisionMeshDirPath);
			for (UStaticMesh* Mesh : CollisionMeshes)
			{
				UE_LOG(LogAGX, Warning, TEXT("  %s"), *Mesh->GetPathName());
				AssetsToDelete.AddUnique(Mesh);
			}
		}

		// Delete removed Render Materials.
		UE_LOG(LogAGX, Warning, TEXT(""));
		UE_LOG(LogAGX, Warning, TEXT("Render Materials:"));
		if (auto* ModelSourceComponent =
				Cast<UAGX_ModelSourceComponent>(SCSNodes.ModelSourceComponent->ComponentTemplate))
		{
			// Find all Render Material assets currently in the import folder.
			const FString RenderMaterialDirPath = GetImportDirPath(
				Helper, FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName());
			TArray<UMaterialInstanceConstant*> Assets =
				FAGX_EditorUtilities::FindAssets<UMaterialInstanceConstant>(RenderMaterialDirPath);
			UE_LOG(LogAGX, Warning, TEXT("Found the following Render Material assets:"));
			for (const UMaterialInstanceConstant* Asset : Assets)
			{
				const FString GuidStr = [&]()
				{
					const FGuid* const AssetGuidPtr =
						ModelSourceComponent->UnrealMaterialToImportGuid.Find(Asset->GetPathName());
					if (AssetGuidPtr != nullptr)
					{
						return AssetGuidPtr->ToString();
					}
					else
					{
						return FString(TEXT("(Unknown)"));
					}
				}();
				UE_LOG(
					LogAGX, Warning, TEXT("  %s associated with GUID %s."), *Asset->GetPathName(),
					*GuidStr);
			}

			// Find all Render Materials that are about to be imported.
			UE_LOG(LogAGX, Warning, TEXT("Collecting Render Material simulation objects:"))
			TSet<FGuid> InSimulation;
			auto CollectFromSimulation = [&InSimulation](const auto& ShapeBarriers)
			{
				UE_LOG(LogAGX, Warning, TEXT("  Collecting from a shape type."));
				for (const auto& ShapeBarrier : ShapeBarriers)
				{
					UE_LOG(
						LogAGX, Warning, TEXT("    Checking shape %s."), *ShapeBarrier.GetName());
					if (!ShapeBarrier.HasRenderMaterial())
					{
						UE_LOG(
							LogAGX, Warning,
							TEXT("    Does not have a Render Material, nothing to add"));
						continue;
					}

					FAGX_RenderMaterial Material = ShapeBarrier.GetRenderMaterial();
					UE_LOG(
						LogAGX, Warning,
						TEXT("    Has Render Material %s, adding to In Simulation set."),
						*Material.Guid.ToString());
					InSimulation.Add(Material.Guid);
				}
			};
			auto CollectFromSimulationBodies = [&InSimulation, &CollectFromSimulation](
												   const TArray<FRigidBodyBarrier>& BodyBarriers)
			{
				UE_LOG(LogAGX, Warning, TEXT("  Collecting from Rigid Bodies."));
				for (const FRigidBodyBarrier& BodyBarrier : BodyBarriers)
				{
					CollectFromSimulation(BodyBarrier.GetShapes());
				}
			};
			CollectFromSimulation(SimulationObjects.GetBoxShapes());
			CollectFromSimulation(SimulationObjects.GetCapsuleShapes());
			CollectFromSimulation(SimulationObjects.GetCylinderShapes());
			CollectFromSimulation(SimulationObjects.GetSphereShapes());
			CollectFromSimulation(SimulationObjects.GetTrimeshShapes());
			CollectFromSimulationBodies(SimulationObjects.GetRigidBodies());

			// Mark for deletion any asset we currently have but don't want to keep.
			UE_LOG(LogAGX, Warning, TEXT("Marking removed assets for deletion."));
			for (auto* Asset : Assets)
			{
				UE_LOG(LogAGX, Warning, TEXT("Checking %s."), *Asset->GetPathName());
				const FGuid* const AssetGuidPtr =
					ModelSourceComponent->UnrealMaterialToImportGuid.Find(Asset->GetPathName());
				if (AssetGuidPtr == nullptr)
				{
					// This is a new, unknown, material.
					// I don't think this is supposed to happen.
					// If the user create new
					UE_LOG(LogAGX, Warning, TEXT("  Unknown asset, ignoring."));
					continue;
				}
				const FGuid AssetGuid = *AssetGuidPtr;
				UE_LOG(LogAGX, Warning, TEXT("  Has GUID %s."), *AssetGuid.ToString());
				if (!InSimulation.Contains(AssetGuid))
				{
					// Found an asset that is no longer part of the model. Delete it.
					AssetsToDelete.Add(Asset);
					ModelSourceComponent->UnrealMaterialToImportGuid.Remove(Asset->GetPathName());
					UE_LOG(LogAGX, Warning, TEXT("  Not among the simulation objects, removing."));
				}
				else
				{
					UE_LOG(LogAGX, Warning, TEXT("  Found among the simulation objects, leaving."));
				}
			}
		}

		// Delete removed Shape Materials.
		{
			// Create a quick-lookup collection of the Shape Materials we want to keep.
			TSet<FGuid> InSimulation;
			for (const FShapeMaterialBarrier& Barrier : SimulationObjects.GetShapeMaterials())
			{
				InSimulation.Add(Barrier.GetGuid());
			}

			// Find all Shape Material assets currently in the import folder, some of which we
			// want to remove.
			const FString ShapeMaterialDirPath = GetImportDirPath(
				Helper, FAGX_ImportUtilities::GetImportShapeMaterialDirectoryName());
			TArray<UAGX_ShapeMaterial*> Assets =
				FAGX_EditorUtilities::FindAssets<UAGX_ShapeMaterial>(ShapeMaterialDirPath);

			// Mark for deletion any asset that we don't want to keep, i.e. that isn't among the
			// simulation objects.
			for (UAGX_ShapeMaterial* Asset : Assets)
			{
				const FGuid Guid = Asset->ImportGuid;
				if (Guid == FGuid())
				{
					// This isn't an imported asset but something the user put there themselves.
					// Don't touch.
					continue;
				}

				if (!InSimulation.Contains(Guid))
				{
					AssetsToDelete.Add(Asset);
				}
			}
		}

		FAGX_EditorUtilities::DeleteImportedAssets(AssetsToDelete);
	}

	// Removes Components that are not present in the new SimulationObjectCollection, meaning they
	// were deleted from the source file since the previous import. The passed SCSNodes will also
	// be kept up to date, i.e. elements removed from BaseBP will have their corresponding SCS Node
	// removed from SCSNodes as well.
	void RemoveDeletedComponents(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects,
		const FAGX_SynchronizeModelSettings& Settings)
	{
		const FShapeGuidsCollection NewShapeGuids = GetShapeGuids(SimulationObjects);

		RemoveDeletedStaticMeshComponents(BaseBP, SCSNodes, NewShapeGuids, Settings);
		RemoveDeletedShapes(BaseBP, SCSNodes, Settings, NewShapeGuids);
		RemoveDeletedRigidBodies(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedConstraints(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedTireModels(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedObserverFrames(BaseBP, SCSNodes, SimulationObjects);
	}

	// Set unset/unique names on (almost) all SCS Nodes to avoid future name collisions during
	// synchronization.
	void SetUnnamedNameForPossibleCollisions(SCSNodeCollection& SCSNodes)
	{
		auto SetUnnamedName = [](USCS_Node* Node)
		{
			if (Node == nullptr)
				return;
			Node->SetVariableName(*FAGX_ImportUtilities::GetUnsetUniqueImportName());
		};

		auto SetUnnamedNameForAll = [&SetUnnamedName](TMap<FGuid, USCS_Node*>& Nodes)
		{
			for (auto& NodeTuple : Nodes)
			{
				SetUnnamedName(NodeTuple.Value);
			}
		};

		SetUnnamedNameForAll(SCSNodes.RigidBodies);
		SetUnnamedNameForAll(SCSNodes.SphereShapes);
		SetUnnamedNameForAll(SCSNodes.BoxShapes);
		SetUnnamedNameForAll(SCSNodes.CylinderShapes);
		SetUnnamedNameForAll(SCSNodes.CapsuleShapes);
		SetUnnamedNameForAll(SCSNodes.TrimeshShapes);
		SetUnnamedNameForAll(SCSNodes.HingeConstraints);
		SetUnnamedNameForAll(SCSNodes.PrismaticConstraints);
		SetUnnamedNameForAll(SCSNodes.BallConstraints);
		SetUnnamedNameForAll(SCSNodes.CylindricalConstraints);
		SetUnnamedNameForAll(SCSNodes.DistanceConstraints);
		SetUnnamedNameForAll(SCSNodes.LockConstraints);
		SetUnnamedNameForAll(SCSNodes.TwoBodyTires);
		SetUnnamedNameForAll(SCSNodes.ObserverFrames);

		SetUnnamedName(SCSNodes.CollisionGroupDisablerComponent);
		SetUnnamedName(SCSNodes.ContactMaterialRegistrarComponent);
		SetUnnamedName(SCSNodes.ModelSourceComponent);

		// Important note: it turns out that calling USCS_Node::SetVariableName is extremely slow
		// performance wise, taking tens of milliseconds. For large models it is not uncommon that
		// we get several hundreds of Components in the Blueprint after an import. This means we are
		// spending a huge amount of time simply renaming SCS Nodes during the model
		// synchronization. Therefore, we can to some optimizations here where we can avoid setting
		// unnamed names for some Components that we can prove is safe to omit renaming. This is
		// done for some Component types below with explanations.

		// USCSNodes.RenderStaticMeshComponents - omitted.
		// USCSNodes.CollisionStaticMeshComponents - omitted.

		// We can omit the RenderStaticMeshComponents and CollisionStaticMeshComponents because
		// those contain the actual GUID itself in their names after import and thus they are by
		// definition already unique and cannot give rise to naming collision during model
		// synchronization. To ensure this stays true also in the future, this is tested in an
		// internal Unit test, which also refers back to this text from a comment in it.

		// We purposely omit renaming the root component since that will never be updated.
		// We also do not rename Track or Wire Components since they are currently not supported for
		// synchronization.
	}

	bool SynchronizeModel(UBlueprint& BaseBP, const FAGX_SynchronizeModelSettings& Settings)
	{
		FScopedSlowTask ImportTask(100.f, LOCTEXT("SynchronizeModel", "Synchronizing Model"), true);
		ImportTask.MakeDialog();
		ImportTask.EnterProgressFrame(5.f, FText::FromString("Collecting data"));

		SCSNodeCollection SCSNodes(BaseBP);
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadAGXArchive(Settings.FilePath, SimObjects))
		{
			return false;
		}

		if (!EnsureSameSource(SCSNodes, SimObjects))
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Could not match this Blueprint with the selected file. None of the objects "
				"of the selected file matched those of the original import. Please ensure the "
				"selected file corresponds to the content of this "
				"Blueprint.");
			return false;
		}

		FAGX_SimObjectsImporterHelper Helper(
			Settings.FilePath, Settings.bIgnoreDisabledTrimeshes,
			GetModelDirectoryFromAsset(&BaseBP));

		ImportTask.EnterProgressFrame(5.0f, FText::FromString("Deleting old assets"));
		DeleteRemovedAssets(BaseBP, SCSNodes, SimObjects, Helper, Settings);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Deleting old Components"));
		RemoveDeletedComponents(BaseBP, SCSNodes, SimObjects, Settings);

		// This overwrites all (supported) Node names with temporary names.
		// We do this since old to-be-removed or to-be-renamed Nodes may "block" the availability of
		// a certain name (all Node names must be unique) that would otherwise be used for a new
		// Component name. This would make the result of a Model Synchronization non-deterministic
		// in terms of Node naming.
		SetUnnamedNameForPossibleCollisions(SCSNodes);

		ImportTask.EnterProgressFrame(
			80.f, FText::FromString("Adding and Updating Components and Assets"));
		AddOrUpdateAll(BaseBP, SCSNodes, SimObjects, Settings, Helper);

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Finalizing Synchronization"));

		// Model synchronization is completed, we end by compiling and saving the Blueprint and any
		// children.
		FAGX_EditorUtilities::SaveAndCompile(BaseBP);

		return true;
	}
}

bool AGX_ImporterToBlueprint::SynchronizeModel(
	UBlueprint& BaseBP, const FAGX_SynchronizeModelSettings& Settings)
{
	if (!AGX_ImporterToBlueprint_SynchronizeModel_helpers::SynchronizeModel(BaseBP, Settings))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Some issues occurred during model synchronization. Log category LogAGX in the Console "
			"may contain more information.",
			"Synchronize model");
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
