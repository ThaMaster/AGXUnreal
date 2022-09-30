// Copyright 2022, Algoryx Simulation AB.

#include "AGX_ImporterToBlueprint.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImportEnums.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SimObjectsImporterHelper.h"
#include "AGX_UrdfImporterHelper.h"
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
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "SimulationObjectCollection.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

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

	FString CreateBlueprintPackagePath(FAGX_SimObjectsImporterHelper& Helper)
	{
		// Create directory for this import and a "Blueprints" directory inside of that.
		/// \todo I think this is more complicated than it needs to be. What are all the pieces for?
		FString ParentPackagePath =
			FAGX_ImportUtilities::CreatePackagePath(Helper.DirectoryName, TEXT("Blueprint"));
		FString ParentAssetName = Helper.SourceFileName;
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(ParentPackagePath, ParentAssetName);

#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* ParentPackage = CreatePackage(nullptr, *ParentPackagePath);
#else
		UPackage* ParentPackage = CreatePackage(*ParentPackagePath);
#endif

		FString Path = FPaths::GetPath(ParentPackage->GetName());

		UE_LOG(
			LogAGX, Display, TEXT("File '%s' imported to package '%s', path '%s'"),
			*Helper.SourceFileName, *ParentPackagePath, *Path);

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName = TEXT("BP_") + Helper.DirectoryName;
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

	void ClearOwningActors(UAGX_ConstraintComponent* Constraint)
	{
		if (Constraint == nullptr)
		{
			return;
		}

		// By default the BodyAttachments are created with the OwningActor set to the owner of
		// the RigidBodyComponents passed to CreateConstraintComponent. In this case the
		// OwningActor points to the temporary template Actor from which the Blueprint is
		// created. By setting them to nullptr instead we restore the constructor / Class
		// Default Object value which won't be serialized and PostInitProperties in the final
		// ConstraintComponent will set OwningActor to GetTypedOuter<AActor>() which is correct
		// in the Blueprint case.
		Constraint->BodyAttachment1.RigidBody.OwningActor = nullptr;
		Constraint->BodyAttachment2.RigidBody.OwningActor = nullptr;
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
		bool Success = true;
		for (const auto& ContactMaterial : SimObjects.GetContactMaterials())
		{
			Success &= Helper.InstantiateContactMaterial(ContactMaterial, ImportedActor) != nullptr;
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
			Success &= Helper.InstantiateTrimesh(Shape, ImportedActor) != nullptr;
		}

		return Success;
	}

	bool AddRigidBodyAndAnyOwnedShape(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper, FScopedSlowTask& ImportTask, float WorkAvalable)
	{
		const int32 NumBodies = SimObjects.GetRigidBodies().Num();
		if (NumBodies == 0)
		{
			return true;
		}

		const float SingleBodyWork = WorkAvalable / static_cast<float>(NumBodies);
		const FText TaskBaseText = ImportTask.GetCurrentMessage();

		bool Success = true;
		for (const FRigidBodyBarrier& Body : SimObjects.GetRigidBodies())
		{
			const FText TaskText = FText::FromString(TaskBaseText.ToString() + Body.GetName());
			ImportTask.EnterProgressFrame(SingleBodyWork, TaskText);
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
				Success &= Helper.InstantiateTrimesh(Trimesh, ImportedActor, &Body) != nullptr;
			}
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
			Helper.InstantiateTwoBodyTire(Tire, ImportedActor, true);
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
			ClearOwningActors(Hinge);
			Success &= Hinge != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetPrismaticConstraints())
		{
			auto Prismatic = Helper.InstantiatePrismatic(Constraint, ImportedActor);
			ClearOwningActors(Prismatic);
			Success &= Prismatic != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetBallConstraints())
		{
			auto BallConstraint= Helper.InstantiateBallJoint(Constraint, ImportedActor);
			ClearOwningActors(BallConstraint);
			Success &= BallConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetCylindricalConstraints())
		{
			auto CylindricalConstraint = Helper.InstantiateCylindricalJoint(Constraint, ImportedActor);
			ClearOwningActors(CylindricalConstraint);
			Success &= CylindricalConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetDistanceConstraints())
		{
			auto DistanceConstraint = Helper.InstantiateDistanceJoint(Constraint, ImportedActor);
			ClearOwningActors(DistanceConstraint);
			Success &= DistanceConstraint != nullptr;
		}

		for (const auto& Constraint : SimObjects.GetLockConstraints())
		{
			auto LockConstraint = Helper.InstantiateLockJoint(Constraint, ImportedActor);
			ClearOwningActors(LockConstraint);
			Success &= LockConstraint != nullptr;
		}

		return Success;
	}

	bool AddDisabledCollisionGroups(
		AActor& ImportedActor, const FSimulationObjectCollection& SimObjects,
		FAGX_SimObjectsImporterHelper& Helper)
	{
		return Helper.InstantiateCollisionGroupDisabler(
				   ImportedActor, SimObjects.GetDisabledCollisionGroups()) != nullptr;
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
						   ObserverFr.Name, ObserverFr.BodyGuid, ObserverFr.Transform,
						   ImportedActor) != nullptr;
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

		const float WorkImportBodies = 50.f;
		ImportTask.EnterProgressFrame(
			10.f, FText::FromString("Reading Rigid Body and its Shapes: "));
		Success &= AddRigidBodyAndAnyOwnedShape(
			ImportedActor, SimObjects, Helper, ImportTask, WorkImportBodies);

		ImportTask.EnterProgressFrame(50.f, FText::FromString("Reading bodiless Shapes"));
		Success &= AddBodilessShapes(ImportedActor, SimObjects, Helper);

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

		ImportTask.EnterProgressFrame(5.f, FText::FromString("Import complete"));
		return Success;
	}

	bool AddComponentsFromAGXArchive(AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper)
	{
		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadAGXArchive(Helper.SourceFilePath, SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Some issues occurred during import. Log category LogAGX in the Console may "
				"contain "
				"more information.",
				"Import AGX Dynamics archive to Blueprint");
			return false;
		}

		return true;
	}

	bool AddComponentsFromUrdf(AActor& ImportedActor, FAGX_SimObjectsImporterHelper& Helper)
	{
		FAGX_UrdfImporterHelper* HelperUrdf = static_cast<FAGX_UrdfImporterHelper*>(&Helper);

		FSimulationObjectCollection SimObjects;
		if (!FAGXSimObjectsReader::ReadUrdf(
				HelperUrdf->SourceFilePath, HelperUrdf->UrdfPackagePath, SimObjects) ||
			!AddAllComponents(ImportedActor, SimObjects, Helper))
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Some issues occurred during import. Log category LogAGX in the Console may "
				"contain "
				"more information.",
				"Import AGX Dynamics archive to Blueprint");
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

	UBlueprint* CreateBlueprint(UPackage* Package, AActor* Template, bool OpenBlueprintEditor)
	{
		static constexpr bool ReplaceInWorld = false;
		static constexpr bool KeepMobility = true;
		FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
		Params.bReplaceActor = ReplaceInWorld;
		Params.bKeepMobility = KeepMobility;
		Params.bOpenBlueprint = OpenBlueprintEditor;

		UBlueprint* Blueprint =
			FKismetEditorUtilities::CreateBlueprintFromActor(Package->GetName(), Template, Params);
		check(Blueprint);
		return Blueprint;
	}

	void PostCreationTeardown(
		AActor* Template, UPackage* Package, UBlueprint* Blueprint, const FString& PackagePath)
	{
		Template->Destroy();
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

	UBlueprint* ImportToBlueprint(
		FAGX_SimObjectsImporterHelper& Helper, EAGX_ImportType ImportType,
		bool OpenBlueprintEditor = true)
	{
		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Helper);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		AActor* Template = CreateTemplate(Helper, ImportType);
		if (Template == nullptr)
		{
			return nullptr;
		}
		UBlueprint* Blueprint = CreateBlueprint(Package, Template, OpenBlueprintEditor);
		PostCreationTeardown(Template, Package, Blueprint, BlueprintPackagePath);
		return Blueprint;
	}
}

UBlueprint* AGX_ImporterToBlueprint::ImportAGXArchive(const FString& ArchivePath)
{
	FAGX_SimObjectsImporterHelper Helper(ArchivePath);
	return ImportToBlueprint(Helper, EAGX_ImportType::Agx);
}

UBlueprint* AGX_ImporterToBlueprint::ImportURDF(
	const FString& UrdfFilePath, const FString& UrdfPackagePath)
{
	FAGX_UrdfImporterHelper Helper(UrdfFilePath, UrdfPackagePath);
	return ImportToBlueprint(Helper, EAGX_ImportType::Urdf);
}

#undef LOCTEXT_NAMESPACE
