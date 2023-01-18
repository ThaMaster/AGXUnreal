// Copyright 2022, Algoryx Simulation AB.

#include "AGX_ImporterToBlueprint.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImportEnums.h"
#include "AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_ReImportComponent.h"
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
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
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
			if (Helper.ImportSettings.bIgnoreDisabledTrimeshes && !Shape.GetEnableCollisions())
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
				if (Helper.ImportSettings.bIgnoreDisabledTrimeshes &&
					!Trimesh.GetEnableCollisions())
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
		volatile bool Success = true;

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
		Helper.InstantiateReImportComponent(ImportedActor);
		Helper.FinalizeImport();

		ImportTask.EnterProgressFrame(40.f, FText::FromString("Import complete"));
		return Success;
	}

	bool AddComponentsFromAGXArchive(AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper)
	{
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadAGXArchive(Helper.ImportSettings.FilePath, SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			return false;
		}

		return true;
	}

	bool AddComponentsFromUrdf(AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper)
	{
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadUrdf(
				Helper.ImportSettings.FilePath, Helper.ImportSettings.UrdfPackagePath,
				SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			return false;
		}

		return true;
	}

	AActor* CreateTemplate(FAGX_SimObjectsImporterHelper& Helper, EAGX_ImportType ImportType)
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

		const bool Result = ImportType == EAGX_ImportType::Urdf
								? AddComponentsFromUrdf(*RootActorContainer, Helper)
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
		FAGX_SimObjectsImporterHelper& Helper, EAGX_ImportType ImportType)
	{
		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Helper, true);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		AActor* Template = CreateTemplate(Helper, ImportType);
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
		FAGX_SimObjectsImporterHelper& Helper, EAGX_ImportType ImportType,
		bool OpenBlueprintEditor = true)
	{
		// The result of the import is stored in the BlueprintBase which is placed in the
		// 'Blueprint' directory in the context browser and should never be edited by the user. It
		// is the "original". The BlueprintChild is what the user will interact directly with, and
		// it is a child of the BlueprintBase. This way, we can ensure re-import works as intended.
		UBlueprint* BlueprintBase = ImportToBaseBlueprint(Helper, ImportType);
		if (BlueprintBase == nullptr)
		{
			return nullptr;
		}

		UBlueprint* BlueprintChild = CreateChildBlueprint(BlueprintBase, Helper);

		if (BlueprintChild != nullptr && OpenBlueprintEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(
				BlueprintChild);
		}

		return BlueprintChild;
	}
}

UBlueprint* AGX_ImporterToBlueprint::Import(const FAGX_ImportSettings& ImportSettings)
{
	FAGX_SimObjectsImporterHelper Helper(ImportSettings);
	UBlueprint* Bp = ImportToBlueprint(
		Helper, ImportSettings.ImportType, ImportSettings.bOpenBlueprintEditorAfterImport);
	if (Bp == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Some issues occurred during import. Log category LogAGX in the Console may "
			"contain more information.",
			"Import model to Blueprint");
	}

	return Bp;
}

namespace AGX_ImporterToBlueprint_reimport_helpers
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
			"/Game", FAGX_ImportUtilities::GetImportRootDirectoryName(), Helper.DirectoryName, Subdir);
	}

	struct FShapeGuidsCollection
	{
		// Values of the TMaps indicates collision enabled status.
		TMap<FGuid, bool> PrimitiveShapeGuids;
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
				Infos.PrimitiveShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetBoxShapes())
			{
				Infos.PrimitiveShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetCylinderShapes())
			{
				Infos.PrimitiveShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
				if (auto RenderDataGuid = GetRenderDataGuidFrom(Shape))
					Infos.RenderDataGuids.Add(*RenderDataGuid);
			}
			for (const auto& Shape : Body.GetCapsuleShapes())
			{
				Infos.PrimitiveShapeGuids.Add(Shape.GetShapeGuid(), Shape.GetEnableCollisions());
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
			Infos.PrimitiveShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetBoxShapes())
		{
			Infos.PrimitiveShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetCylinderShapes())
		{
			Infos.PrimitiveShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetCapsuleShapes())
		{
			Infos.PrimitiveShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.RenderDataGuids.Add(*RenderDataGuid);
		}
		for (const auto& Barrier : SimulationObjects.GetTrimeshShapes())
		{
			Infos.TrimeshShapeGuids.Add(Barrier.GetShapeGuid(), Barrier.GetEnableCollisions());
			if (auto RenderDataGuid = GetRenderDataGuidFrom(Barrier))
				Infos.PrimitiveShapeGuids.Add(*RenderDataGuid);
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
				else if (auto Sh = Cast<UAGX_ShapeComponent>(Component))
				{
					AGX_CHECK(!ShapeComponents.Contains(Sh->ImportGuid));
					if (Sh->ImportGuid.IsValid())
						ShapeComponents.Add(Sh->ImportGuid, Node);
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
				else if (auto Re = Cast<UAGX_ReImportComponent>(Component))
				{
					AGX_CHECK(ReImportComponent == nullptr);
					ReImportComponent = Node;
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
					// Handled by gathering information from the ReImportComponent since a Static
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
						LogAGX, Error, TEXT("Found Node: '%s' with unsupported type."),
						*Node->GetName());
					AGX_CHECK(false);
				}
			}
		}

		// The key is the AGX Dynamics object's UUID converted to an FGuid at the time of the
		// previous import.
		TMap<FGuid, USCS_Node*> RigidBodies;
		TMap<FGuid, USCS_Node*> ShapeComponents; // Including Shapes owned by Rigid Bodies.
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
		USCS_Node* ReImportComponent = nullptr;
		USCS_Node* RootComponent = nullptr;
		// @todo Append rest of the types here...
	};

	// Returns true if at least one Guid could be matched, false otherwise.
	bool EnsureSameSource(
		const SCSNodeCollection& SCSNodes, const FSimulationObjectCollection& SimulationObjects)
	{
		const TArray<FGuid> BodyBarrierGuids =
			GetGuidsFromBarriers(SimulationObjects.GetRigidBodies());
		for (auto& NodeTuple : SCSNodes.RigidBodies)
		{
			if (BodyBarrierGuids.Contains(NodeTuple.Key))
			{
				return true;
			}
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
		if (SimulationObjects.GetContactMaterials().Num() == 0)
		{
			return;
		}

		USCS_Node* CMRegistrarNode = GetOrCreateContactMaterialRegistrarNode(BaseBP, SCSNodes);
		CMRegistrarNode->SetVariableName(TEXT("AGX_ContactMaterialRegistrar"));
		auto CMRegistrar =
			Cast<UAGX_ContactMaterialRegistrarComponent>(CMRegistrarNode->ComponentTemplate);

		// Find all existing assets that might be of interest from the previous import.
		const FString ContactMaterialDirPath =
			GetImportDirPath(Helper, FAGX_ImportUtilities::GetImportContactMaterialDirectoryName());
		TMap<FGuid, UAGX_ContactMaterial*> ExistingContactMaterialsMap =
			FindAGXAssetComponents<UAGX_ContactMaterial>(ContactMaterialDirPath);

		for (const auto& Barrier : SimulationObjects.GetContactMaterials())
		{
			const FGuid Guid = Barrier.GetGuid();
			if (ExistingContactMaterialsMap.Contains(Guid))
			{
				Helper.UpdateAndSaveContactMaterialAsset(
					Barrier, *ExistingContactMaterialsMap[Guid], *CMRegistrar);
			}
			else
			{
				Helper.InstantiateContactMaterial(Barrier, *CMRegistrar);
			}
		}
	}

	template <typename TBarrier, typename TComponent>
	USCS_Node* AddOrUpdateShape(
		const TBarrier& Barrier, UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		FAGX_SimObjectsImporterHelper& Helper,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& MSTsOnDisk,
		USCS_Node* OverrideAttachParent = nullptr)
	{
		USCS_Node* AttachParent = OverrideAttachParent != nullptr
									  ? OverrideAttachParent
									  : BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();

		const FGuid Guid = Barrier.GetShapeGuid();
		USCS_Node* Node = nullptr;
		if (SCSNodes.ShapeComponents.Contains(Guid))
		{
			Node = SCSNodes.ShapeComponents[Guid];
			ReParentNode(BaseBP, *Node, *AttachParent);
		}
		else
		{
			Node = BaseBP.SimpleConstructionScript->CreateNode(
				TComponent::StaticClass(), FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			AttachParent->AddChildNode(Node);
			SCSNodes.ShapeComponents.Add(Guid, Node);
		}

		Helper.UpdateComponent(Barrier, *Cast<TComponent>(Node->ComponentTemplate), MSTsOnDisk);
		return Node;
	}

	void AddOrUpdateRenderData(
		const FShapeBarrier& ShapeBarrier, USCS_Node& AttachParent, UBlueprint& BaseBP,
		SCSNodeCollection& SCSNodes, FAGX_SimObjectsImporterHelper& Helper)
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
			*Cast<UStaticMeshComponent>(RenderDataNode->ComponentTemplate));
	}

	void AddOrUpdateRigidBodiesAndOwnedShapes(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_ImportSettings& ImportSettings,
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
				ExistingMSTAssets);

			// Add or update all shapes in the current Rigid Body.
			for (const auto& ShapeBarrier : RbBarrier.GetSphereShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_SphereShapeComponent>(
						ShapeBarrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetBoxShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_BoxShapeComponent>(
						ShapeBarrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetCylinderShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_CylinderShapeComponent>(
						ShapeBarrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetCapsuleShapes())
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(ShapeBarrier), UAGX_CapsuleShapeComponent>(
						ShapeBarrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets, RigidBodyNode);
				AddOrUpdateRenderData(ShapeBarrier, *ShapeNode, BaseBP, SCSNodes, Helper);
			}

			for (const auto& ShapeBarrier : RbBarrier.GetTrimeshShapes())
			{
				USCS_Node* RenderDataParent = nullptr;
				if (!ShapeBarrier.GetEnableCollisions() && ImportSettings.bIgnoreDisabledTrimeshes)
				{
					RenderDataParent = RigidBodyNode;
				}
				else
				{
					USCS_Node* ShapeNode =
						AddOrUpdateShape<decltype(ShapeBarrier), UAGX_TrimeshShapeComponent>(
							ShapeBarrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets,
							RigidBodyNode);

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
						ShapeBarrier,
						*Cast<UStaticMeshComponent>(CollisionMesh->ComponentTemplate));
					RenderDataParent = CollisionMesh;
				}

				AddOrUpdateRenderData(ShapeBarrier, *RenderDataParent, BaseBP, SCSNodes, Helper);
			}
		}
	}

	void AddOrUpdateBodilessShapes(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper,
		const FAGX_ImportSettings& ImportSettings,
		const TMap<FGuid, UAGX_MergeSplitThresholdsBase*>& ExistingMSTAssets)
	{
		for (const auto& Barrier : SimulationObjects.GetSphereShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_SphereShapeComponent>(
				Barrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper);
		}

		for (const auto& Barrier : SimulationObjects.GetBoxShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_BoxShapeComponent>(
				Barrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper);
		}

		for (const auto& Barrier : SimulationObjects.GetCylinderShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_CylinderShapeComponent>(
				Barrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper);
		}

		for (const auto& Barrier : SimulationObjects.GetCapsuleShapes())
		{
			USCS_Node* ShapeNode = AddOrUpdateShape<decltype(Barrier), UAGX_CapsuleShapeComponent>(
				Barrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets);
			AddOrUpdateRenderData(Barrier, *ShapeNode, BaseBP, SCSNodes, Helper);
		}

		for (const auto& Barrier : SimulationObjects.GetTrimeshShapes())
		{
			USCS_Node* RenderDataParent = nullptr;
			if (!Barrier.GetEnableCollisions() && ImportSettings.bIgnoreDisabledTrimeshes)
			{
				RenderDataParent = BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode();
			}
			else
			{
				USCS_Node* ShapeNode =
					AddOrUpdateShape<decltype(Barrier), UAGX_TrimeshShapeComponent>(
						Barrier, BaseBP, SCSNodes, Helper, ExistingMSTAssets);

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
					Barrier, *Cast<UStaticMeshComponent>(CollisionMesh->ComponentTemplate));

				RenderDataParent = CollisionMesh;
			}

			AddOrUpdateRenderData(Barrier, *RenderDataParent, BaseBP, SCSNodes, Helper);
		}

		// @todo: we should clean up old collision groups in instance components. Currently, we
		// always add to them, but never remove. The cleanup must be done after all shapes have been
		// re-imported.
	}

	void AddOrUpdateConstraints(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper)
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
					Barrier, *Cast<UAGX_ConstraintComponent>(Constraint->ComponentTemplate));
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
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper)
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
				Barrier, *Cast<UAGX_TwoBodyTireComponent>(TireNode->ComponentTemplate));
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
				// In the case that no CollisionGroupDisablerComponent existed before the re-import,
				// and there are no disabled Collision Groups in the model being re-imported, there
				// is no need to create a CollisionGroupDisablerComponent. We are done.
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
		const FSimulationObjectCollection& SimulationObjects, FAGX_SimObjectsImporterHelper& Helper)
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
				*Cast<UAGX_ObserverFrameComponent>(ObserverNode->ComponentTemplate));
		}
	}

	void AddOrUpdateReImportComponent(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes, FAGX_SimObjectsImporterHelper& Helper)
	{
		USCS_Node* ReImportComponent = nullptr;
		if (SCSNodes.ReImportComponent == nullptr)
		{
			ReImportComponent = BaseBP.SimpleConstructionScript->CreateNode(
				UAGX_ReImportComponent::StaticClass(),
				FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
			BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode()->AddChildNode(
				ReImportComponent);
			SCSNodes.ReImportComponent = ReImportComponent;
		}
		else
		{
			ReImportComponent = SCSNodes.ReImportComponent;
		}

		Helper.UpdateReImportComponent(
			*Cast<UAGX_ReImportComponent>(ReImportComponent->ComponentTemplate));
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
		const FAGX_ImportSettings& ImportSettings)
	{
		FAGX_SimObjectsImporterHelper Helper(ImportSettings, GetModelDirectoryFromAsset(&BaseBP));

		// We collect all existing merge split thresholds assets here once, and pass it down to the
		// relevant AddOrUpdate functions below, instead of looking it up several times from within
		// those functions.
		const FString MSTDirPath = GetImportDirPath(
			Helper, FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName());
		TMap<FGuid, UAGX_MergeSplitThresholdsBase*> ExistingMSTAssets =
			FindAGXAssetComponents<UAGX_MergeSplitThresholdsBase>(MSTDirPath);

		AddOrUpdateShapeMaterials(BaseBP, SimulationObjects, Helper);
		AddOrUpdateContactMaterials(BaseBP, SCSNodes, SimulationObjects, Helper);
		AddOrUpdateRigidBodiesAndOwnedShapes(
			BaseBP, SCSNodes, SimulationObjects, Helper, ImportSettings, ExistingMSTAssets);
		AddOrUpdateBodilessShapes(
			BaseBP, SCSNodes, SimulationObjects, Helper, ImportSettings, ExistingMSTAssets);
		AddOrUpdateConstraints(BaseBP, SCSNodes, SimulationObjects, Helper);
		AddOrUpdateTwoBodyTires(BaseBP, SCSNodes, SimulationObjects, Helper);
		AddOrUpdateCollisionGroupDisabler(BaseBP, SCSNodes, SimulationObjects, Helper);
		AddOrUpdateObserverFrames(BaseBP, SCSNodes, SimulationObjects, Helper);
		AddOrUpdateReImportComponent(BaseBP, SCSNodes, Helper);

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
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes, const FAGX_ImportSettings& ImportSettings,
		const FShapeGuidsCollection& NewShapeGuids)
	{
		for (auto It = SCSNodes.ShapeComponents.CreateIterator(); It; ++It)
		{
			if (!NewShapeGuids.PrimitiveShapeGuids.Contains(It->Key) &&
				!NewShapeGuids.TrimeshShapeGuids.Contains(It->Key))
			{
				BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
				It.RemoveCurrent();
			}
			else if (NewShapeGuids.TrimeshShapeGuids.Contains(It->Key))
			{
				// If we re-import with the "Ignore disabled Trimeshes" import setting and this
				// Trimesh has collision disabled, it should be removed.
				const bool CollisionEnabled = NewShapeGuids.TrimeshShapeGuids[It->Key];
				if (!CollisionEnabled && ImportSettings.bIgnoreDisabledTrimeshes)
				{
					BaseBP.SimpleConstructionScript->RemoveNodeAndPromoteChildren(It->Value);
					It.RemoveCurrent();
				}
			}
		}
	}

	void RemoveDeletedStaticMeshComponents(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes, const FShapeGuidsCollection& NewShapeGuids,
		const FAGX_ImportSettings& ImportSettings)
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
				// re-import.
				// 2. The AGX Trimesh that is being re-imported has collision disabled.
				if (!ImportSettings.bIgnoreDisabledTrimeshes)
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

	// Removes Components that are not present in the new SimulationObjectCollection, meaning they
	// were deleted from the source file since the previous import. The passed SCSNodes will also
	// be kept up to date, i.e. elements removed from BaseBP will have their corresponding SCS Node
	// removed from SCSNodes as well.
	void RemoveDeletedComponents(
		UBlueprint& BaseBP, SCSNodeCollection& SCSNodes,
		const FSimulationObjectCollection& SimulationObjects,
		const FAGX_ImportSettings& ImportSettings)
	{
		const FShapeGuidsCollection NewShapeGuids = GetShapeGuids(SimulationObjects);

		RemoveDeletedStaticMeshComponents(BaseBP, SCSNodes, NewShapeGuids, ImportSettings);
		RemoveDeletedShapes(BaseBP, SCSNodes, ImportSettings, NewShapeGuids);
		RemoveDeletedRigidBodies(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedConstraints(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedTireModels(BaseBP, SCSNodes, SimulationObjects);
		RemoveDeletedObserverFrames(BaseBP, SCSNodes, SimulationObjects);
	}

	void SetUnnamedNameForAll(UBlueprint& BaseBP)
	{
		for (USCS_Node* Node : BaseBP.SimpleConstructionScript->GetAllNodes())
		{
			if (Node == nullptr)
			{
				continue;
			}

			// Do not rename the root component.
			if (Node == BaseBP.SimpleConstructionScript->GetDefaultSceneRootNode())
			{
				continue;
			}

			if (auto C = Cast<UAGX_WireComponent>(Node->ComponentTemplate))
			{
				// Wires are not supported by the re-import pipeline, meaning we should not make any
				// changes to it.
				continue;
			}

			if (auto C = Cast<UAGX_TrackComponent>(Node->ComponentTemplate))
			{
				// Tracks are not supported by the re-import pipeline, meaning we should not make
				// any changes to it.
				continue;
			}

			Node->SetVariableName(*FAGX_ImportUtilities::GetUnsetUniqueImportName());
		}
	}

	bool ReImport(UBlueprint& BaseBP, const FAGX_ImportSettings& ImportSettings)
	{
		SCSNodeCollection SCSNodes(BaseBP);
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadAGXArchive(ImportSettings.FilePath, SimObjects))
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

		RemoveDeletedComponents(BaseBP, SCSNodes, SimObjects, ImportSettings);

		// This overwrites all (supported) Node names with temporary names.
		// We do this since old to-be-removed or to-be-renamed Nodes may "block" the availability of
		// a certain name (all Node names must be unique) that would otherwise be used for a new
		// Component name. This would make the result of a ReImport non-deterministic in terms of
		// Node naming.
		SetUnnamedNameForAll(BaseBP);
		AddOrUpdateAll(BaseBP, SCSNodes, SimObjects, ImportSettings);

		// Re-import is completed, we end by compiling and saving the Blueprint and any children.
		FAGX_EditorUtilities::SaveAndCompile(BaseBP);

		return true;
	}
}

bool AGX_ImporterToBlueprint::ReImport(
	UBlueprint& BaseBP, const FAGX_ImportSettings& ImportSettings)
{
	if (!AGX_ImporterToBlueprint_reimport_helpers::ReImport(BaseBP, ImportSettings))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Some issues occurred during re-import. Log category LogAGX in the Console may "
			"contain more information.",
			"Re-import model to Blueprint");
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
