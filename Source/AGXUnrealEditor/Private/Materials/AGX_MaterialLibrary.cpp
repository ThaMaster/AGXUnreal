// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_MaterialLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Materials/MaterialLibraryBarrier.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "PackageTools.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/SavePackage.h"

// Naming convention:
//   Name: dirt_1
//   AssetName: AGX_TM_dirt_1.
//   PackagePath: /AGXUnreal/Terrain/TerrainMaterials/
//   AssetPath: /AGXUnreal/Terrain/TerrainMaterials/AGX_TM_dirt_1.AGX_TM_dirt_1

namespace AGX_MaterialLibrary_helpers
{
	void EnsureMaterialImported(const FString& NameAGX)
	{
		// Create a package for our asset.
		const FString Name = UPackageTools::SanitizePackageName(NameAGX);
		const FString AssetName = FString::Printf(TEXT("AGX_TM_%s"), *Name);
		const FString PackagePath =
			FString::Printf(TEXT("/AGXUnreal/Terrain/TerrainMaterialLibrary/%s"), *AssetName);

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());

		// Explicitly delete any existing file. Otherwise save below may fail.
		IFileManager::Get().Delete(*PackageFilename);

#if UE_VERSION_OLDER_THAN(4, 26, 0)
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#else
		UPackage* Package = CreatePackage(*PackagePath);
#endif
		Package->FullyLoad();

		// Create the asset itself, reading data from the AGX Dynamics terrain material library.
		FTerrainMaterialBarrier Material =
			AGX_MaterialLibraryBarrier::LoadMaterialProfile(Name);
		UAGX_TerrainMaterial* Asset = NewObject<UAGX_TerrainMaterial>(
			Package, FName(*AssetName), RF_Public | RF_Standalone);
		Asset->CopyFrom(Material);

		// Do the cargo culting.
		FAssetRegistryModule::AssetCreated(Asset);
		Asset->MarkPackageDirty();
		Asset->PostEditChange();
		Asset->AddToRoot();
		Package->SetDirtyFlag(true);
		Package->FullyLoad();
		Package->GetMetaData();

		// Save the package to disk.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		const bool bSaved = UPackage::SavePackage(Package, Asset, RF_NoFlags, *PackageFilename);
#else
		FSavePackageArgs saveArgs;
		saveArgs.TopLevelFlags = RF_NoFlags;
		const bool bSaved = UPackage::SavePackage(Package, Asset, *PackageFilename, saveArgs);
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
		// unnecessary ones once we know which can safely be removed.
		Package->FullyLoad();
	}
}

void AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary()
{
	using namespace AGX_MaterialLibrary_helpers;

	const TArray<FString> Names = AGX_MaterialLibraryBarrier::GetAvailableLibraryMaterials();
	for (const FString& NameAGX : Names)
	{
		EnsureMaterialImported(NameAGX);
	}
}
