#include "AGX_ArchiveImporterToBlueprint.h"

#include "AGX_LogCategory.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "FileHelpers.h"
#include "AssetSelection.h"
#include "AssetToolsModule.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
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

		int32 TryCount = 0;
		while (PackageExists(PackagePath))
		{
			++TryCount;
			PackagePath = BasePackagePath + "_" + FString::FromInt(TryCount);
			check(TryCount < 10000); /// \todo For debugging only. Remove.
		};

		return FBlueprintId(ArchiveFilename, PackagePath, BlueprintName);
	}

	UPackage* GetPackage(const FBlueprintId& BlueprintId)
	{
		UPackage* Package = CreatePackage(nullptr, *BlueprintId.PackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	AActor* CreateTemplate(const FString& BlueprintName)
	{
		UActorFactory* Factory =
			GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
		FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
		UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
		AActor* RootActorContainer =
			FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
		check(RootActorContainer != nullptr);
		USceneComponent* ActorRootComponent = NewObject<USceneComponent>(
			RootActorContainer, USceneComponent::GetDefaultSceneRootVariableName());
		check(ActorRootComponent != nullptr);
		ActorRootComponent->Mobility = EComponentMobility::Movable;
		ActorRootComponent->bVisualizeComponent = true;
		RootActorContainer->SetRootComponent(ActorRootComponent);
		RootActorContainer->AddInstanceComponent(ActorRootComponent);
		ActorRootComponent->RegisterComponent();
		RootActorContainer->SetActorLabel(BlueprintName);
		RootActorContainer->SetFlags(RF_Transactional);
		ActorRootComponent->SetFlags(RF_Transactional);

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
	AActor* Template = CreateTemplate(Id.BlueprintName);
	UBlueprint* Blueprint = CreateBlueprint(Package, Template);
	PostCreationTeardown(Package, Blueprint, Id.PackagePath);
	return Blueprint;
}
