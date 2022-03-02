// Copyright 2022, Algoryx Simulation AB.


#include "Materials/AGX_TerrainMaterialLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_TerrainMaterialAsset.h"
#include "Materials/TerrainMaterialLibraryBarrier.h"

// Unreal Engine includes.
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/SavePackage.h"

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
		const FString Name = UPackageTools::SanitizePackageName(NameAGX);
		const FString AssetName = FString::Printf(TEXT("AGX_TM_%s"), *Name);
		const FString PackagePath =
			FString::Printf(TEXT("/AGXUnreal/Terrain/TerrainMaterialLibrary/%s"), *AssetName);
#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
		UPackage* Package = CreatePackage(*PackagePath);
#endif
		Package->FullyLoad();

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
		Package->FullyLoad();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
		Package->GetMetaData();

		// Save the package to disk.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		const bool bSaved = UPackage::SavePackage(Package, Asset, RF_NoFlags, *PackageFilename);
#else
		/// @todo [UE5] Really no save args here?
		const bool bSaved = UPackage::SavePackage(Package, Asset, *PackageFilename, FSavePackageArgs());
#endif
		if (!bSaved)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create terrain library material %s: UPackage::SavePackage failed."),
				*NameAGX);
		}

		// Must fully load the package or else project packaging will fail with:
		//
		//    Package /AGXUnreal/Terrain/TerrainMaterialLibrary/AGX_TM_gravel_1 supposed
		//    to be fully loaded but isn't. RF_WasLoaded is set
		//
		//    Unable to cook package for platform because it is unable to be loaded:
		//    <PATH>/AGXUnreal/Content/Terrain/TerrainMaterialLibrary/AGX_TM_gravel_1.uasset
		//
		// I'm not entirely sure where the FullyLoad call should be for it to
		// take effect in all cases, so there are a few of them. Remove the
		// uncessary ones once we know which can safely be removed.
		Package->FullyLoad();
	}
}

void AGX_TerrainMaterialLibrary::InitializeTerrainMaterialAssetLibrary()
{
	using namespace AGX_TerrainMaterialLibrary_helpers;

	const TArray<FString> Names = AGX_TerrainMaterialLibraryBarrier::GetAvailableLibraryMaterials();
	for (const FString& NameAGX : Names)
	{
		EnsureMaterialImported(NameAGX);
	}
}
