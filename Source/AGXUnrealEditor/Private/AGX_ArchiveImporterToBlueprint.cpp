#include "AGX_ArchiveImporterToBlueprint.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ArchiveImporterHelper.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGXArchiveReader.h"
#include "CollisionGroups/AGX_DisabledCollisionGroupsComponent.h"
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
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ImportUtilities.h"

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
#include "UObject/Package.h"
#include "PackageTools.h"

namespace
{
	void PreCreationSetup()
	{
		GEditor->SelectNone(false, false);
	}

	FString CreateBlueprintPackagePath(FAGX_ArchiveImporterHelper& Helper)
	{
		// Create directory for this archive and a "Blueprints" directory inside of that.
		/// \todo I think this is more complicated than it needs to be. What are all the pieces for?
		FString ParentPackagePath =
			FAGX_ImportUtilities::CreateArchivePackagePath(Helper.DirectoryName, TEXT("Blueprint"));
		FString ParentAssetName = Helper.ArchiveFileName; /// \todo Why is this never used?
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(ParentPackagePath, ParentAssetName);
		UPackage* ParentPackage = CreatePackage(nullptr, *ParentPackagePath);
		FString Path = FPaths::GetPath(ParentPackage->GetName());

		UE_LOG(
			LogAGX, Display, TEXT("Archive '%s' imported to package '%s', path '%s'"),
			*Helper.ArchiveFileName, *ParentPackagePath, *Path);

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName = TEXT("BP_") + Helper.DirectoryName;
		FString BasePackagePath = UPackageTools::SanitizePackageName(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		auto PackageExists = [](const FString& PackagePath) {
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
		UPackage* Package = CreatePackage(nullptr, *BlueprintPackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	class FBlueprintBody final : public FAGXArchiveBody
	{
	public:
		FBlueprintBody(UAGX_RigidBodyComponent& InBody, FAGX_ArchiveImporterHelper& InHelper)
			: Body(InBody)
			, Helper(InHelper)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
		{
			Helper.InstantiateSphere(Barrier, Body);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
		{
			Helper.InstantiateBox(Barrier, Body);
		}

		virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
		{
			Helper.InstantiateCylinder(Barrier, Body);
		}

		virtual void InstantiateCapsule(const FCapsuleShapeBarrier& Barrier) override
		{
			Helper.InstantiateCapsule(Barrier, Body);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
		{
			Helper.InstantiateTrimesh(Barrier, Body);
		}

	private:
		UAGX_RigidBodyComponent& Body;
		FAGX_ArchiveImporterHelper& Helper;
	};

	class FBlueprintInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		FBlueprintInstantiator(AActor& InBlueprintTemplate, FAGX_ArchiveImporterHelper& InHelper)
			: Helper(InHelper)
			, BlueprintTemplate(InBlueprintTemplate)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component = Helper.InstantiateBody(Barrier, BlueprintTemplate);
			if (Component == nullptr)
			{
				return new NopEditorBody();
			}
			// This is the attach part of the RootComponent strangeness. I would like to call
			// AttachToComponent here, but I don't have a RootComponent. See comment in
			// CreateTemplate.
#if 0
			Component->AttachToComponent(
				BlueprintTemplate->GetRootComponent(),
				FAttachmentTransformRules::KeepWorldTransform);
#endif
			return new FBlueprintBody(*Component, Helper);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiateHinge(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiatePrismatic(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiateBallJoint(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiateCylindricalJoint(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiateDistanceJoint(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& Barrier) override
		{
			UAGX_ConstraintComponent* Constraint =
				Helper.InstantiateLockJoint(Barrier, BlueprintTemplate);
			ClearOwningActors(Constraint);
		}

		void ClearOwningActors(UAGX_ConstraintComponent* Constraint)
		{
			if (Constraint == nullptr)
			{
				return;
			}

			// By default the BodyAttachments are created with the OwningActor set to the owner of
			// the RigidBodyComponents passed to CreateConstraintComponent. In this case the
			// OwningActor points to the temporary template actor and Unreal doesn't update the
			// pointers to instead point to the actor that is created when the Blueprint is
			// instantiated. The best we can do is to set them to nullptr and rely on the body
			// names only.
			Constraint->BodyAttachment1.RigidBody.OwningActor = nullptr;
			Constraint->BodyAttachment2.RigidBody.OwningActor = nullptr;
		}

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledPairs) override
		{
			if (DisabledPairs.Num() == 0)
			{
				return;
			}

			UAGX_DisabledCollisionGroupsComponent* Component =
				NewObject<UAGX_DisabledCollisionGroupsComponent>(
					&BlueprintTemplate, TEXT("DisabledCollisionGroupPairs"));
			Component->SetFlags(RF_Transactional);
			BlueprintTemplate.AddInstanceComponent(Component);
			Component->RegisterComponent();
			for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
			{
				Component->DisabledCollisionGroupPairs.Add(
					{FName(*DisabledPair.first), FName(*DisabledPair.second)});
			}
		}

		virtual void InstantiateShapeMaterial(const FShapeMaterialBarrier& Barrier) override
		{
			Helper.InstantiateShapeMaterial(Barrier);
		}

		virtual void InstantiateContactMaterial(const FContactMaterialBarrier& Barrier) override
		{
			Helper.InstantiateContactMaterial(Barrier);
		}

		virtual FTwoBodyTireArchiveBodies InstantiateTwoBodyTire(
			const FTwoBodyTireBarrier& Barrier) override
		{
			// Instantiate the Tire and Hub Rigid Bodies. This adds them to the RestoredBodies TMap
			// and can thus be found and used when the TwoBodyTire component is instantiated.
			const FRigidBodyBarrier TireBody = Barrier.GetTireRigidBody();
			const FRigidBodyBarrier HubBody = Barrier.GetHubRigidBody();
			if (TireBody.HasNative() == false || HubBody.HasNative() == false)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("At lest one of the Rigid Bodies referenced by the TwoBodyTire %s did not "
						 "have a native Rigid Body. The TwoBodyTire will not be instantiated."),
					*Barrier.GetName());
				return FTwoBodyTireArchiveBodies(new NopEditorBody(), new NopEditorBody());
			}

			FTwoBodyTireArchiveBodies ArchiveBodies;
			ArchiveBodies.TireBodyArchive.reset(InstantiateBody(TireBody));
			ArchiveBodies.HubBodyArchive.reset(InstantiateBody(HubBody));

			Helper.InstantiateTwoBodyTire(Barrier, BlueprintTemplate, true);
			return ArchiveBodies;
		}

		virtual ~FBlueprintInstantiator() = default;

	private:
		using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;
		using FShapeMaterialPair = std::pair<UAGX_ShapeMaterialAsset*, UAGX_ShapeMaterialAsset*>;

	private:
		FAGX_ArchiveImporterHelper Helper;
		AActor& BlueprintTemplate;
	};

	void AddComponentsFromArchive(AActor& ImportedActor, FAGX_ArchiveImporterHelper& Helper)
	{
		FBlueprintInstantiator Instantiator(ImportedActor, Helper);
		FAGXArchiveReader::Read(Helper.ArchiveFilePath, Instantiator);
	}

	AActor* CreateTemplate(FAGX_ArchiveImporterHelper& Helper)
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

		AddComponentsFromArchive(*RootActorContainer, Helper);
		return RootActorContainer;
	}

	UBlueprint* CreateBlueprint(UPackage* Package, AActor* Template)
	{
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprintFromActor(
			Package->GetName(), Template, false, true);
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

		UPackage::SavePackage(
			Package, Blueprint, RF_Public | RF_Standalone | RF_MarkAsRootSet, *PackageFilename,
			GError, nullptr, true, true, SAVE_NoError);
	}
}

UBlueprint* AGX_ArchiveImporterToBlueprint::ImportAGXArchive(const FString& ArchivePath)
{
	FAGX_ArchiveImporterHelper Helper(ArchivePath);
	PreCreationSetup();
	FString BlueprintPackagePath = CreateBlueprintPackagePath(Helper);
	UPackage* Package = GetPackage(BlueprintPackagePath);
	AActor* Template = CreateTemplate(Helper);
	UBlueprint* Blueprint = CreateBlueprint(Package, Template);
	PostCreationTeardown(Template, Package, Blueprint, BlueprintPackagePath);
	return Blueprint;
}
