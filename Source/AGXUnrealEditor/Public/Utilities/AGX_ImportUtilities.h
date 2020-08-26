#pragma once

// AGXUnreal includes.
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class FTrimeshShapeBarrier;
class FShapeBarrier;
class FShapeMaterialBarrier;
class FContactMaterialBarrier;
class UAGX_ContactMaterialAsset;
class UAGX_ShapeMaterialAsset;
struct FAGX_RenderMaterial;

class AActor;
class FString;
class UActorComponent;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UStaticMesh;

class FAGX_ImportUtilities
{
public:
	/**
	 * Create a package path for an asset of the given type. The returned path is the sanitized
	 * version of "/Game/ImportedAgxArchives/{ArchiveName}/{AssetType}s/".
	 * @param ArchiveName The name of the archive from which the asset was read.
	 * @param AssetType The type of the asset.
	 * @return A package path for the asset, or the empty string if the names are invalid.
	 */
	static FString CreateArchivePackagePath(FString ArchiveName, FString AssetType);

	/**
	 * Create a package path for an imported AGX Dynamics archive with the given name.
	 *
	 * The given name is sanitized and the returned package path will then be
	 * "/Game/ImportedAgxArchives/{ArchiveName}".
	 *
	 * No check is made for already existing packages with the same name.
	 *
	 * @param ArchiveName The name of the AGX Dynamics archive to create a package path for.
	 * @return The package path for the AGX Dynamics archive.
	 */
	static FString CreateArchivePackagePath(FString ArchiveName);

	/**
	 * Pick a name for an imported asset. NativeName and ArchiveName will be sanitized and the first
	 * non-empty of the two is returned. If both sanitize to the empty string then AssetType is
	 * returned unchanged. Even though the name returned will be valid, it may not be unique and may
	 * therefore not be the final asset name.
	 * @param NativeName The name of the restored object.
	 * @param ArchiveName The of the archive from which the native object was read.
	 * @param AssetType The type of the asset.
	 * @return A safe name for the asset.
	 */
	static FString CreateAssetName(
		const FString& NativeName, const FString& FallbackName, const FString& AssetType);

	/**
	 * Find package- and asset names that are unique. The package name can be a directory. A unique
	 * name, based on AssetName, will be generated and the full package- and asset names will be
	 * stored in the parameters.
	 *
	 * An example, passing "/Game/Textures/", "MyTexture" when there already is an asset named
	 * "MyTexture" in "/Game/Textures/" will result in "/Game/Textures/MyTexture_1" to be stored in
	 * PackageName and "MyTexture_1" to be stored in AssetName.
	 *
	 * @param PackageName Package path to the folder that should hold the new asset.
	 * @param AssetName Candidate name for the new asset.
	 */
	static void MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName);

	/// \todo Determine if it's enough to return the asset created in the following few
	/// functions, or if we must pack it in a struct together with the package path and/or asset
	/// name.

	/**
	 * Store an AGX Dynamics Trimesh imported from an AGX Dynamics archive as a UStaticMesh
	 * asset.
	 * @param Trimesh The imported trimesh to be saved.
	 * @param DirectoryName The name of the directory where the archive's assets are collected.
	 * @param FallbackName Name to give the asset in case the trimesh doesn't have a source
	 * name.
	 * @return The UStaticMesh asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName,
		const FString& FallbackName);

	/**
	 * Store an AGX Dynamics Material imported from an AGX Dynamics archive as an
	 * UAGX_ShapeMaterialAsset.
	 * @param Material The imported material to be saved.
	 * @param DirectoryName The name of the archive from which the material was read.
	 * @return The created ShapeMaterialAsset.
	 */
	static UAGX_ShapeMaterialAsset* SaveImportedShapeMaterialAsset(
		const FShapeMaterialBarrier& Material, const FString& DirectoryName);

	/**
	 * Store an AGX Dynamics ContactMaterial imported from an AGX Dynamics archive as an
	 * UAGX_ContactMaterialAsset.
	 * @param ContactMaterial The imported contact material to be saved.
	 * @param Material1 The AGXUnreal ShapeMaterial for the first AGX Dynamics material.
	 * @param Material2 The AGXUnreal ShapeMaterial for the second AGX Dynamics material.
	 * @param DirectoryName The Name of the archive from which the material was read.
	 * @return The created ContactMaterialAsset.
	 */
	static UAGX_ContactMaterialAsset* SaveImportedContactMaterialAsset(
		const FContactMaterialBarrier& ContactMaterial, UAGX_ShapeMaterialAsset* Material1,
		UAGX_ShapeMaterialAsset* Material2, const FString& DirectoryName);

	/**
	 * Save an FAGX_RenderMaterial read from and AGX Dynamics RenderData material as an Unreal
	 * Engine Material Instance. The Material Instance will be inheriting from the base import
	 * material M_ImportedBase that is shipped as an asset with the AGX Dynamics for Unreal plugin.
	 * The base material is returned if a Material Instance could not be created, and nullptr is
	 * returned if the base material could not be loaded.
	 * @param Imported AGX Dynamics Render Material parameters.
	 * @param DirectoryName Name where assets for the imported AGX Dynamics archive should be
	 * stored. Often the same as the archive itself.
	 * @param MaterialName The name to give to the new Material Instance.
	 * @return A new Material Instance if one could be created, or the base material, or
	 * nullptr if the base material could not be loaded.
	 */
	static UMaterialInterface* SaveImportedRenderMaterialAsset(
		const FAGX_RenderMaterial& Imported, const FString& DirectoryName,
		const FString& MaterialName);

	/**
	 * Rename the object. Generates a fallback name if the given name can't be used.
	 */
	static void Rename(UObject& Object, const FString& Name);
};
