#include "AGX_ArchiveImporterToBlueprint.h"

// Unreal Engine includes.
#include "AGXArchiveReader.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "HingeBarrier.h"
#include "PrismaticBarrier.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

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
	IAssetTools& GetAssetTools()
	{
		return FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	}

	void PreCreationSetup()
	{
		GEditor->SelectNone(false, false);
	}

	FString Sanitize(const FString& In)
	{
		return UPackageTools::SanitizePackageName(In);
	}

	struct FBlueprintId
	{
		// FString ArchiveFilepath;
		FString ArchiveFilename;
		FString PackagePath;
		FString BlueprintName;

		FBlueprintId(
			const FString& InArchiveFilename, const FString& InPackagePath,
			const FString& InBlueprintName)
			: ArchiveFilename(InArchiveFilename)
			, PackagePath(InPackagePath)
			, BlueprintName(InBlueprintName)
		{
		}
	};

	FBlueprintId CreateBlueprintId(const FString& ArchiveFilepath)
	{
		const FString ArchiveFilename = FPaths::GetBaseFilename(ArchiveFilepath);

		FString ParentPackageName = TEXT("/Game/ImportedBlueprints/");
		FString ParentAssetName = ArchiveFilename;
		IAssetTools& AssetTools = GetAssetTools();
		AssetTools.CreateUniqueAssetName(
			ParentPackageName, ParentAssetName, ParentPackageName, ParentAssetName);
		UPackage* ParentPackage = CreatePackage(nullptr, *ParentPackageName);
		FString Path = FPaths::GetPath(ParentPackage->GetName());

		const FString BlueprintName = TEXT("BP_") + ArchiveFilename;
		FString BasePackagePath = Sanitize(Path + "/" + BlueprintName);
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
			check(TryCount < 10000); /// \todo For debugging only. Remove.
		}

		return FBlueprintId(ArchiveFilename, PackagePath, BlueprintName);
	}

	UPackage* GetPackage(const FBlueprintId& BlueprintId)
	{
		UPackage* Package = CreatePackage(nullptr, *BlueprintId.PackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	class FBlueprintBody final : public FAGXArchiveBody
	{
	public:
		FBlueprintBody(UAGX_RigidBodyComponent* InBodyComponent)
			: BodyComponent(InBodyComponent)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
		{
			UAGX_SphereShapeComponent* Component =
				FAGX_EditorUtilities::CreateSphereShape(BodyComponent->GetOwner(), BodyComponent);
			Component->Radius = Barrier.GetRadius();
			FinalizeShape(Component, Barrier);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
		{
			UAGX_BoxShapeComponent* Component =
				FAGX_EditorUtilities::CreateBoxShape(BodyComponent->GetOwner(), BodyComponent);
			Component->HalfExtent = Barrier.GetHalfExtents();
			FinalizeShape(Component, Barrier);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Trimesh) override
		{
		}

	private:
		void FinalizeShape(UAGX_ShapeComponent* Component, const FShapeBarrier& Barrier)
		{
			Component->bCanCollide = Barrier.GetEnableCollisions();
			for (const FName& Group : Barrier.GetCollisionGroups())
			{
				Component->AddCollisionGroup(Group);
			}
			Component->SetRelativeLocation(Barrier.GetLocalPosition());
			Component->SetRelativeRotation(Barrier.GetLocalRotation());
			Component->UpdateVisualMesh();
			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				Name = MakeUniqueObjectName(Component->GetOwner(), Component->GetClass(), *Name)
						   .ToString();
			}
			Component->Rename(*Name);
		}

	private:
		UAGX_RigidBodyComponent* BodyComponent;
	};

	class FBlueprintInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		FBlueprintInstantiator(AActor* InBlueprintTemplate)
			: BlueprintTemplate(InBlueprintTemplate)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component =
				NewObject<UAGX_RigidBodyComponent>(BlueprintTemplate, NAME_None);

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				Name = MakeUniqueObjectName(
						   BlueprintTemplate, UAGX_RigidBodyComponent::StaticClass(), *Name)
						   .ToString();
			}
			Component->Rename(*Name);

			Component->SetWorldLocation(Barrier.GetPosition());
			Component->SetWorldRotation(Barrier.GetRotation());
			Component->Mass = Barrier.GetMass();
			Component->MotionControl = Barrier.GetMotionControl();

			Component->SetFlags(RF_Transactional);
			BlueprintTemplate->AddInstanceComponent(Component);
			Component->RegisterComponent();

// This is the attach part of the RootComponent strangeness. I would like to
// call AttachToComponent here, but I don't have a RootComponent. See comment in
// CreateTemplate.
#if 0
			Component->AttachToComponent(
				BlueprintTemplate->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
#endif
			Component->PostEditChange();
			RestoredBodies.Add(Barrier.GetGuid(), Component);
			return new FBlueprintBody(Component);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			FBodyPair Bodies = GetBodies(Barrier);
			if (Bodies.first == nullptr)
			{
				// Not having a second body is fine, means that the first body is constrained to the
				// world. Not having a first body is bad.
				UE_LOG(
					LogAGX, Warning, TEXT("Constraint '%s' does not have a first body. Ignoring."),
					*Barrier.GetName());
				return;
			}

			UAGX_HingeConstraintComponent* Component =
				FAGX_EditorUtilities::CreateHingeConstraintComponent(
					BlueprintTemplate, Bodies.first, Bodies.second);
			if (Component == nullptr)
			{
				return;
			}

			// By default the BodyAttachments are created with the OwningActor set to the owner of
			// the RigidBodyComponents passed to CreateConstraintComponent. In this case the
			// OwningActor points to the temporary template actor and Unreal doesn't update the
			// pointers to instead point to the actor that is created when the Blueprint is
			// instantiated. The best we can do is to set them to nullptr and rely on the body
			// names only.
			Component->BodyAttachment1.RigidBody.OwningActor = nullptr;
			Component->BodyAttachment2.RigidBody.OwningActor = nullptr;

			StoreFrames(Barrier, *Component);

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(BlueprintTemplate, Component->GetClass(), FName(*Name))
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Constraint '%s' imported with name '%s' because of name collision."),
					*OldName, *Name);
			}
			Component->Rename(*Name);
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			FBodyPair Bodies = GetBodies(Barrier);
			if (Bodies.first == nullptr)
			{
				// Not having a second body is fine, means that the first body is constrained to the
				// world. Not having a first body is bad.
				UE_LOG(
					LogAGX, Warning, TEXT("Constraint '%s' does not have a first body. Ignoring"),
					*Barrier.GetName());
				return;
			}

			UClass* ConstraintClass = UAGX_PrismaticConstraintComponent::StaticClass();
			UAGX_PrismaticConstraintComponent* Component =
				FAGX_EditorUtilities::CreatePrismaticConstraintComponent(
					BlueprintTemplate, Bodies.first, Bodies.second);
			if (Component == nullptr)
			{
				return;
			}

			Component->BodyAttachment1.RigidBody.OwningActor = nullptr;
			Component->BodyAttachment2.RigidBody.OwningActor = nullptr;

			StoreFrames(Barrier, *Component);
			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(BlueprintTemplate, Component->GetClass(), FName(*Name))
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Constraint'%s' imported with name '%s' because of name collision."),
					*OldName, *Name);
			}
			Component->Rename(*Name);
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& BallJoint) override
		{
		}

		virtual void InstantiateCylindricalJoint(
			const FCylindricalJointBarrier& CylindricalJoint) override
		{
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& DistanceJoint) override
		{
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& LockJoint) override
		{
		}

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledGroups) override
		{
		}

		virtual ~FBlueprintInstantiator() = default;

	private:
		using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;

	private:
		UAGX_RigidBodyComponent* GetBody(const FRigidBodyBarrier& Barrier)
		{
			if (!Barrier.HasNative())
			{
				// Not an error. Means constrained with world.
				return nullptr;
			}
			FGuid Guid = Barrier.GetGuid();
			UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Guid);
			if (Component == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found a constraint to body '%s', but that body isn't known."),
					*Barrier.GetName());
				return nullptr;
			}
			return Component;
		}

		FBodyPair GetBodies(const FConstraintBarrier& Constraint)
		{
			return {GetBody(Constraint.GetFirstBody()), GetBody(Constraint.GetSecondBody())};
		}

		/// \todo The two StoreFrame(s) member functions are copy/paste from AGX_ArchiveImporter.
		/// Find a reasonable shared location to put them in.

		void StoreFrame(
			const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment,
			int32 BodyIndex)
		{
			Attachment.FrameDefiningActor = nullptr;
			Attachment.LocalFrameLocation = Barrier.GetLocalLocation(BodyIndex);
			Attachment.LocalFrameRotation = Barrier.GetLocalRotation(BodyIndex);
		}

		void StoreFrames(const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component)
		{
			StoreFrame(Barrier, Component.BodyAttachment1, 0);
			StoreFrame(Barrier, Component.BodyAttachment2, 1);
		}

	private:
		AActor* BlueprintTemplate;
		TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
	};

	void AddComponentsFromArchive(const FString& ArchivePath, AActor* ImportedActor)
	{
		FBlueprintInstantiator Instantiator(ImportedActor);
		FAGXArchiveReader::Read(ArchivePath, Instantiator);
	}

	AActor* CreateTemplate(const FString& BlueprintName, const FString ArchivePath)
	{
		UActorFactory* Factory =
			GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
		FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
		UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
		AActor* RootActorContainer =
			FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
		check(RootActorContainer != nullptr);
		RootActorContainer->SetFlags(RF_Transactional);
		RootActorContainer->SetActorLabel(BlueprintName);

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

#if 1
		AddComponentsFromArchive(ArchivePath, RootActorContainer);
#else
		/*
		 * This is the part where the imported objects should be created.
		 */

		// Create static mesh.
		UStaticMeshComponent* StaticMeshComponent =
			NewObject<UStaticMeshComponent>(RootActorContainer, NAME_None);
		StaticMeshComponent->Mobility = EComponentMobility::Movable;
		StaticMeshComponent->SetFlags(RF_Transactional);
		FString StaticMeshUniqueName = TEXT("MyMesh");
		if (!StaticMeshComponent->Rename(*StaticMeshUniqueName, nullptr, REN_Test))
		{
			StaticMeshUniqueName =
				MakeUniqueObjectName(
					RootActorContainer, USceneComponent::StaticClass(), FName("MyMesh"))
					.ToString();
		}
		StaticMeshComponent->Rename(*StaticMeshUniqueName, nullptr, REN_DontCreateRedirectors);
		RootActorContainer->AddInstanceComponent(StaticMeshComponent);
		StaticMeshComponent->RegisterComponent();
		StaticMeshComponent->AttachToComponent(
			ActorRootComponent, FAttachmentTransformRules::KeepWorldTransform);
		StaticMeshComponent->SetRelativeTransform(FTransform());
		StaticMeshComponent->PostEditChange();

		// Create point light.
		UPointLightComponent* LightComponent =
			NewObject<UPointLightComponent>(RootActorContainer, "MyLight");
		LightComponent->SetAttenuationRadius(1000.0f);
		LightComponent->SetIntensity(500.0f);
		LightComponent->SetLightColor(FLinearColor::Green);
		LightComponent->SetCastDeepShadow(true);
		LightComponent->SetFlags(RF_Transactional);
		FString LightUniqueName = TEXT("MyLight");
		if (!LightComponent->Rename(*LightUniqueName, nullptr, REN_Test))
		{
			LightUniqueName =
				MakeUniqueObjectName(
					RootActorContainer, USceneComponent::StaticClass(), FName("MyLight"))
					.ToString();
		}
		LightComponent->Rename(*LightUniqueName, nullptr, REN_DontCreateRedirectors);
		RootActorContainer->AddInstanceComponent(LightComponent);
		LightComponent->RegisterComponent();
		LightComponent->AttachToComponent(
			ActorRootComponent, FAttachmentTransformRules::KeepWorldTransform);
		LightComponent->SetRelativeTransform(FTransform());
		LightComponent->PostEditChange();
#endif

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

		UE_LOG(
			LogAGX, Log, TEXT("Got filename '%s' for package path '%s'."), *PackageFilename,
			*PackagePath);

		UPackage::SavePackage(
			Package, Blueprint, RF_Public | RF_Standalone | RF_MarkAsRootSet, *PackageFilename,
			GError, nullptr, true, true, SAVE_NoError);
	}
}

UBlueprint* AGX_ArchiveImporterToBlueprint::ImportAGXArchive(const FString& ArchivePath)
{
	PreCreationSetup();
	FBlueprintId Id = CreateBlueprintId(ArchivePath);
	UPackage* Package = GetPackage(Id);
	AActor* Template = CreateTemplate(Id.BlueprintName, ArchivePath);
	UBlueprint* Blueprint = CreateBlueprint(Package, Template);
	PostCreationTeardown(Template, Package, Blueprint, Id.PackagePath);
	return Blueprint;
}
