#pragma once

// AGXUnreal includes.
#include "AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class FTrimeshShapeBarrier;
class FShapeMaterialBarrier;
class FContactMaterialBarrier;
class UAGX_ContactMaterialAsset;
class UAGX_ShapeMaterialAsset;

class AActor;
class UActorComponent;
class FString;
class UStaticMesh;

class FAGX_ImportUtilities
{
public:
	/**
	 * Create a package path for an asset of the given type. The returned path is the sanitized
	 * version of "/Game/ImportedAgxArchives/{ArchiveName}/{AssetType}s/"
	 * @param ArchiveName - The name of the archive from which the asset was read.
	 * @param AssetType - The type of the asset.
	 * @return
	 */
	static FString CreateArchivePackagePath(FString ArchiveName, FString AssetType);

	/**
	 * Pick a name for an imported asset. NativeName and ArchiveName will be sanitized and the first
	 * non-empty of the two is returned. If both sanitize to the empty string then AssetType is
	 * returned unchanged. Even though the name returned will be valid, it may not be unique and may
	 * therefore not be the final asset name.
	 * @param NativeName - The name of the restored object.
	 * @param ArchiveName - The of the archive from which the native object was read.
	 * @param AssetType - The type of the asset.
	 * @return A safe name for the asset.
	 */
	static FString CreateAssetName(
		const FString& NativeName, const FString& FallbackName, const FString& AssetType);

	/// \todo Determine if it's enough to return the asset created in the following few
	/// functions, or if we must pack it in a struct together with the package path and/or asset
	/// name.

	/**
	 * Store an AGX Dynamics Trimesh imported from an AGX Dynamics archive as a UStaticMesh
	 * asset.
	 * @param Trimesh - The imported trimesh to be saved.
	 * @param ArchiveName - The name of the archive from which the trimesh was read.
	 * @param FallbackName - Name to give the asset in case the trimesh doesn't have a source
	 * name.
	 * @return
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& ArchiveName,
		const FString& FallbackName);

	/**
	 * Store an AGX Dynamics shape imported from an AGX Dynamics archive as a
	 * UAGX_ShapeMaterialAsset.
	 * @param Material - The imported material to be saved.
	 * @param ArchiveName - The name of the archive from which the material was read.
	 * @return
	 */
	static UAGX_ShapeMaterialAsset* SaveImportedShapeMaterialAsset(
		const FShapeMaterialBarrier& Material, const FString& ArchiveName);

	static UAGX_ContactMaterialAsset* SaveImportedContactMaterialAsset(
		const FContactMaterialBarrier& ContactMaterial, UAGX_ShapeMaterialAsset* Material1,
		UAGX_ShapeMaterialAsset* Material2, const FString& ArchiveName);

	/** Rename the object. Generates a fallback name if the given name can't be used. */
	static void Rename(UObject& Object, const FString& Name);
};
