#include "AGX_ArchiveImporterToBlueprint.h"

// Unreal Engine includes.
#include "AGXArchiveReader.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
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

		virtual void InstantiateSphere(const FSphereShapeBarrier& Sphere) override
		{
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Box) override
		{
			UAGX_BoxShapeComponent* BoxComponent =
				FAGX_EditorUtilities::CreateBoxShape(BodyComponent->GetOwner(), BodyComponent);
			BoxComponent->HalfExtent = Box.GetHalfExtents();

			BoxComponent->bCanCollide = Box.GetEnableCollisions();
			for (const FName& Group : Box.GetCollisionGroups())
			{
				BoxComponent->AddCollisionGroup(Group);
			}
			BoxComponent->SetRelativeLocation(Box.GetLocalPosition());
			BoxComponent->SetRelativeRotation(Box.GetLocalRotation());
			BoxComponent->UpdateVisualMesh();
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Trimesh) override
		{
		}

	private:
		UAGX_RigidBodyComponent* BodyComponent;
	};

	class FBlueprintInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		FBlueprintInstantiator(AActor* InImportedActor)
			: ImportedActor(InImportedActor)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& RigidBody) override
		{
			UAGX_RigidBodyComponent* BodyComponent =
				NewObject<UAGX_RigidBodyComponent>(ImportedActor, NAME_None);

			BodyComponent->SetWorldLocation(RigidBody.GetPosition());
			BodyComponent->SetWorldRotation(RigidBody.GetRotation());
			BodyComponent->Mass = RigidBody.GetMass();
			BodyComponent->MotionControl = RigidBody.GetMotionControl();

			BodyComponent->SetFlags(RF_Transactional);
			ImportedActor->AddInstanceComponent(BodyComponent);
			BodyComponent->RegisterComponent();

// This is the attach part of the RootComponent strangeness. I would like to
// call AttachToComponent here, but I don't have a RootComponent. See comment in
// CreateTemplate.
#if 0
			BodyComponent->AttachToComponent(
				ImportedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
#endif
			BodyComponent->PostEditChange();
			return new FBlueprintBody(BodyComponent);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Hinge) override
		{
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Prismatic) override
		{
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
		AActor* ImportedActor;
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
			Package->GetName(), Template, true, true);
		check(Blueprint);
		return Blueprint;
	}

	void PostCreationTeardown(UPackage* Package, UBlueprint* Blueprint, const FString& PackagePath)
	{
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
	PostCreationTeardown(Package, Blueprint, Id.PackagePath);
	return Blueprint;
}
