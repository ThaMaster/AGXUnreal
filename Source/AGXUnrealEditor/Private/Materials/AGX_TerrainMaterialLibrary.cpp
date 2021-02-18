#include "Materials/AGX_TerrainMaterialLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_TerrainMaterialAsset.h"
#include "TerrainMaterialLibraryBarrier.h"

// Unreal Engine includes.
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "ModuleManager.h"
#include "PackageTools.h"

// Naming convention:
//   Name: dirt_1
//   AssetName: AGX_TM_dirt_1.
//   PackagePath: /AGXUnreal/Terrain/TerrainMaterials/
//   AssetPath: /AGXUnreal/Terrain/TerrainMaterials/AGX_TM_dirt_1.AGX_TM_dirt_1

namespace AGX_TerrainMaterialLibrary_helpers
{
	void EnsureMaterialImported(const FString& NameAGX)
	{
		// Create a package for our asset.
		FString Name = UPackageTools::SanitizePackageName(NameAGX);
		FString AssetName = FString::Printf(TEXT("AGX_TM_%s"), *Name);
		FString PackagePath =
			FString::Printf(TEXT("/AGXUnreal/Terrain/TerrainMaterials/%s"), *AssetName);
		UPackage* Package = CreatePackage(nullptr, *PackagePath);

// Not sure if or when this is needed or legal.
#if 0
		Package->FullyLoad();
#endif

		// Create the asset itself, reading data from the AGX Dynamics terrain material library.
		FTerrainMaterialBarrier Material =
			AGX_TerrainMaterialLibraryBarrier::LoadMaterialProfile(Name);
		UAGX_TerrainMaterialAsset* Asset = NewObject<UAGX_TerrainMaterialAsset>(
			Package, FName(*AssetName), RF_Public | RF_Standalone);
		Asset->CopyFrom(Material);

		// Do the cargo culting.
		FAssetRegistryModule::AssetCreated(Asset);
		Asset->MarkPackageDirty();
		Asset->PostEditChange();
		Asset->AddToRoot();
		Package->SetDirtyFlag(true);
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
		Package->GetMetaData();

		// Save the package to disk.
		if (!UPackage::SavePackage(Package, Asset, RF_NoFlags, *PackageFilename))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create terrain library material %s: UPackage::SavePackage failed."),
				*NameAGX);
		}
	}
}

void AGX_TerrainMaterialLibrary::InitializeTerrainMaterialAssetLibrary()
{
	using namespace AGX_TerrainMaterialLibrary_helpers;

	TArray<FString> Names = AGX_TerrainMaterialLibraryBarrier::GetAvailableLibraryMaterials();
	for (const FString& NameAGX : Names)
	{
		EnsureMaterialImported(NameAGX);
	}
}
