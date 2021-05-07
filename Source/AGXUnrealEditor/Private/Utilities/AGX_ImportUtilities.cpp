#include "Utilities/AGX_ImportUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_ContactMaterialAsset.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "RawMesh.h"

namespace
{
	/// \todo Determine if it's enough to return the created asset, or if we must pack it in a
	/// struct together with the package path and/or asset name.
	template <typename UAsset, typename FInitAssetCallback>
	UAsset* SaveImportedAsset(
		const FString& DirectoryName, FString AssetName, const FString& FallbackName,
		const FString& AssetType, FInitAssetCallback InitAsset)
	{
		AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, FallbackName, AssetType);
		FString PackagePath =
			FAGX_ImportUtilities::CreateArchivePackagePath(DirectoryName, AssetType);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif
		UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		if (!Asset)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Could not create asset '%s' from archive '%s'."), *AssetName,
				*DirectoryName);
			return nullptr;
		}
		InitAsset(*Asset);
		if (!FAGX_EditorUtilities::FinalizeAndSavePackage(Package, Asset, PackagePath, AssetName))
		{
			return nullptr;
		}
		return Asset;
	}
}

FString FAGX_ImportUtilities::CreateArchivePackagePath(FString ArchiveName, FString AssetType)
{
	ArchiveName = FAGX_EditorUtilities::SanitizeName(ArchiveName);
	AssetType = FAGX_EditorUtilities::SanitizeName(AssetType);
	if (ArchiveName.IsEmpty() || AssetType.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/ImportedAgxArchives/%s/%ss/"), *ArchiveName, *AssetType);
}

FString FAGX_ImportUtilities::CreateArchivePackagePath(FString ArchiveName)
{
	ArchiveName = FAGX_EditorUtilities::SanitizeName(ArchiveName);
	if (ArchiveName.IsEmpty())
	{
		return FString();
	}
	return FString::Printf(TEXT("/Game/ImportedAgxArchives/%s"), *ArchiveName);
}

FString FAGX_ImportUtilities::CreateAssetName(
	const FString& NativeName, const FString& FallbackName, const FString& AssetType)
{
	FString Name = FAGX_EditorUtilities::SanitizeName(NativeName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	Name = FAGX_EditorUtilities::SanitizeName(FallbackName);
	if (!Name.IsEmpty())
	{
		return Name;
	}
	return AssetType;
}

void FAGX_ImportUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	FString WantedPackageName = PackageName;
	FString WantedAssetName = AssetName;
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
	if (AssetName != WantedAssetName)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Asset '%s' imported with name '%s' because of name conflict."),
			*WantedAssetName, *AssetName);
	}
}

UStaticMesh* FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName, const FString& FallbackName)
{
	auto InitAsset = [&](UStaticMesh& Asset) {
		FRawMesh RawMesh = FAGX_EditorUtilities::CreateRawMeshFromTrimesh(Trimesh);
		FAGX_EditorUtilities::AddRawMeshToStaticMesh(RawMesh, &Asset);
		Asset.ImportVersion = EImportStaticMeshVersion::LastVersion;
	};
	UStaticMesh* CreatedAsset = SaveImportedAsset<UStaticMesh>(
		DirectoryName, Trimesh.GetSourceName(), FallbackName, TEXT("StaticMesh"), InitAsset);
	return CreatedAsset;
}

UStaticMesh* FAGX_ImportUtilities::SaveImportedStaticMeshAsset(
	const FRenderDataBarrier& RenderData, const FString& DirectoryName)
{
	auto InitAsset = [&](UStaticMesh& Asset) {
		FRawMesh RawMesh = FAGX_EditorUtilities::CreateRawMeshFromRenderData(RenderData);
		FAGX_EditorUtilities::AddRawMeshToStaticMesh(RawMesh, &Asset);
		Asset.ImportVersion = EImportStaticMeshVersion::LastVersion;
	};
	UStaticMesh* CreatedAsset = SaveImportedAsset<UStaticMesh>(
		DirectoryName, FString::Printf(TEXT("RenderMesh_%s"), *RenderData.GetGuid().ToString()),
		TEXT("RenderMesh"), TEXT("RenderMesh"), InitAsset);
	return CreatedAsset;
}

UAGX_ShapeMaterialAsset* FAGX_ImportUtilities::SaveImportedShapeMaterialAsset(
	const FShapeMaterialBarrier& Material, const FString& DirectoryName)
{
	auto InitAsset = [&](UAGX_ShapeMaterialAsset& Asset) { Asset.CopyFrom(&Material); };
	UAGX_ShapeMaterialAsset* CreatedAsset = SaveImportedAsset<UAGX_ShapeMaterialAsset>(
		DirectoryName, Material.GetName(), TEXT(""), TEXT("ShapeMaterial"), InitAsset);
	return CreatedAsset;
}

namespace
{
	FString GetName(UAGX_ShapeMaterialAsset* Material)
	{
		if (Material == nullptr)
		{
			return TEXT("Default");
		}
		return Material->GetName();
	}
}

UAGX_ContactMaterialAsset* FAGX_ImportUtilities::SaveImportedContactMaterialAsset(
	const FContactMaterialBarrier& ContactMaterial, UAGX_ShapeMaterialAsset* Material1,
	UAGX_ShapeMaterialAsset* Material2, const FString& DirectoryName)
{
	const FString Name = TEXT("CM") + GetName(Material1) + GetName(Material2);

	auto InitAsset = [&](UAGX_ContactMaterialAsset& Asset) {
		Asset.CopyFrom(&ContactMaterial);
		Asset.Material1 = Material1;
		Asset.Material2 = Material2;
	};

	UAGX_ContactMaterialAsset* Asset = SaveImportedAsset<UAGX_ContactMaterialAsset>(
		DirectoryName, Name, TEXT(""), TEXT("ContactMaterial"), InitAsset);

	return Asset;
}

UMaterialInterface* FAGX_ImportUtilities::SaveImportedRenderMaterialAsset(
	const FAGX_RenderMaterial& Imported, const FString& DirectoryName, const FString& MaterialName)
{
	UMaterial* Base = LoadObject<UMaterial>(
		nullptr, TEXT("Material'/AGXUnreal/Runtime/Materials/M_ImportedBase.M_ImportedBase'"));
	if (Base == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not load parent material for imported AGX Dynamics render materials."));
		return nullptr;
	}
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = Base;

	FString AssetName = FAGX_ImportUtilities::CreateAssetName(
		MaterialName, TEXT("ImportedAGXDynamicsMaterial"), TEXT("RenderMaterial"));
	FString PackagePath =
		FAGX_ImportUtilities::CreateArchivePackagePath(DirectoryName, TEXT("RenderMaterial"));

	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackagePath, AssetName, PackagePath, AssetName);
	UObject* Asset = AssetTools.CreateAsset(
		AssetName, FPackageName::GetLongPackagePath(PackagePath),
		UMaterialInstanceConstant::StaticClass(), Factory);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material asset for material '%s' imported from '%s' "
				 "Falling back to the default imported material."),
			*MaterialName, *DirectoryName);
		return Base;
	}

	UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Asset);
	if (Material == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not create new Material Instance Constant for material '%s' imported from "
				 "'%s'. Falling back to the default imported material."),
			*MaterialName, *DirectoryName)
		return Base;
	}

	if (Imported.bHasDiffuse)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Diffuse")), FAGX_RenderMaterial::ConvertToLinear(Imported.Diffuse));
	}
	if (Imported.bHasAmbient)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Ambient")), FAGX_RenderMaterial::ConvertToLinear(Imported.Ambient));
	}
	if (Imported.bHasEmissive)
	{
		Material->SetVectorParameterValueEditorOnly(
			FName(TEXT("Emissive")), FAGX_RenderMaterial::ConvertToLinear(Imported.Emissive));
	}
	if (Imported.bHasShininess)
	{
		FMaterialParameterInfo Info;
		Info.Name = TEXT("Shininess");
		Material->SetScalarParameterValueEditorOnly(Info, Imported.Shininess);
	}

	Material->SetFlags(RF_Standalone);
	Material->MarkPackageDirty();
	Material->PostEditChange();
	UPackage* Package = Material->GetTypedOuter<UPackage>();
	if (Package != nullptr)
	{
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, Material, RF_NoFlags, *PackageFilename);
	}

	return Material;
}

void FAGX_ImportUtilities::Rename(UObject& Object, const FString& Name)
{
	if (Name.IsEmpty())
	{
		// Not having an imported name means use whatever default name Unreal decided.
		return;
	}
	if (Object.Rename(*Name, nullptr, REN_Test))
	{
		Object.Rename(*Name, nullptr, REN_DontCreateRedirectors);
	}
	else
	{
		FName NewName = MakeUniqueObjectName(Object.GetOuter(), Object.GetClass(), FName(*Name));
		UE_LOG(
			LogAGX, Warning, TEXT("%s '%s' imported with name '%s' because of name conflict."),
			*Object.GetClass()->GetName(), *Name, *NewName.ToString());
		Object.Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
	}
}

FLinearColor FAGX_ImportUtilities::SRGBToLinear(const FVector4& SRGB)
{
	FColor SRGBBytes(
		static_cast<uint8>(SRGB.X * 255.0f), static_cast<uint8>(SRGB.Y * 255.0f),
		static_cast<uint8>(SRGB.Z * 255.0f), static_cast<uint8>(SRGB.W * 255.0f));
	return {SRGBBytes};
}

FVector4 FAGX_ImportUtilities::LinearToSRGB(const FLinearColor& Linear)
{
	FColor SRGBBytes = Linear.ToFColor(true);
	return FVector4(
		static_cast<float>(SRGBBytes.R) / 255.0f, static_cast<float>(SRGBBytes.G) / 255.0f,
		static_cast<float>(SRGBBytes.B) / 255.0f, static_cast<float>(SRGBBytes.A) / 255.0f);
}
