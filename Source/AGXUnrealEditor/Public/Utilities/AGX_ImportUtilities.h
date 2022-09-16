// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_AmorEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Color.h"

class FTrimeshShapeBarrier;
class FRenderDataBarrier;
class FShapeBarrier;
class FMergeSplitThresholdsBarrier;
class FShapeMaterialBarrier;
class FContactMaterialBarrier;
class UAGX_ContactMaterialAsset;
class UAGX_MergeSplitThresholdsBase;
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
	 * version of "/Game/ImportedAGXModels/{FileName}/{AssetType}s/".
	 * @param FileName The name of the source file from which the asset was read.
	 * @param AssetType The type of the asset.
	 * @return A package path for the asset, or the empty string if the names are invalid.
	 */
	static FString CreatePackagePath(FString FileName, FString AssetType);

	/**
	 * Create a package path for imported simulation objects with the given name.
	 *
	 * The given name is sanitized and the returned package path will then be
	 * "/Game/ImportedAGXModels/{FileName}".
	 *
	 * No check is made for already existing packages with the same name.
	 *
	 * @param FileName The name of the source file to create a package path for.
	 * @return The package path for the imported asset.
	 */
	static FString CreatePackagePath(FString FileName);

	/**
	 * Pick a name for an imported asset. NativeName and FileName will be sanitized and the first
	 * non-empty of the two is returned. If both sanitize to the empty string then AssetType is
	 * returned unchanged. Even though the name returned will be valid, it may not be unique and may
	 * therefore not be the final asset name.
	 * @param NativeName The name of the restored object.
	 * @param FileName The name of the source file from which the asset was read.
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
	/// /Save.+Asset/ functions, or if we must pack it in a struct together with the package path
	/// and/or asset name.

	/**
	 * Store the imported Trimesh as an UStaticMesh asset.
	 *
	 * @param Trimesh The imported trimesh to be saved.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @param FallbackName Name to give the asset in case the trimesh doesn't have a source
	 * name.
	 * @return The created UStaticMesh asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName,
		const FString& FallbackName);

	/**
	 * Store the imported Render Data Mesh as an UStaticMesh
	 * asset.
	 *
	 * @param RenderData The Render Data holding the render mesh to store.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @return The create UStaticMesh asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FRenderDataBarrier& RenderData, const FString& DirectoryName);

	/**
	 * Store an imported AGX Dynamics Material as an UAGX_ShapeMaterialAsset.
	 * @param Material The imported material to be saved.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @return The created ShapeMaterialAsset.
	 */
	static UAGX_ShapeMaterialAsset* SaveImportedShapeMaterialAsset(
		const FShapeMaterialBarrier& Material, const FString& DirectoryName);

	/**
	 * Store an imported AGX Dynamics ContactMaterial as an UAGX_ContactMaterialAsset.
	 * @param ContactMaterial The imported contact material to be saved.
	 * @param Material1 The AGXUnreal ShapeMaterial for the first AGX Dynamics material.
	 * @param Material2 The AGXUnreal ShapeMaterial for the second AGX Dynamics material.
	 * @param DirectoryName The name of the directory where the assets are collected.
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
	 * returned if the base material could not be loaded. The passed MaterialName is used if
	 * possible, but a sequence number is added, using IAssetTools::CreateUniqueAssetName, in case
	 * of a name conflict.
	 * @param Imported AGX Dynamics Render Material parameters.
	 * @param DirectoryName Name where assets for the imported assets should be
	 * stored. Often the same as the source filename itself.
	 * @param MaterialName The name to give to the new Material Instance. A sequence number will be
	 * added in case of a conflict
	 * @return A new Material Instance if one could be created, or the base material, or
	 * nullptr if the base material could not be loaded.
	 */
	static UMaterialInterface* SaveImportedRenderMaterialAsset(
		const FAGX_RenderMaterial& Imported, const FString& DirectoryName,
		const FString& MaterialName);

	static UAGX_MergeSplitThresholdsBase* SaveImportedMergeSplitAsset(
		const FMergeSplitThresholdsBarrier& Barrier,
		EAGX_AmorOwningType OwningType, const FString& DirectoryName,
		const FString& Name);

	/**
	 * Rename the object. Generates a fallback name if the given name can't be used.
	 */
	static void Rename(UObject& Object, const FString& Name);

	/**
	 * Handles the case of renaming Actor Components, where an extra name validation occurs compared
	 * to the more general Rename(UObject&, ...) version of this function.
	 */
	static void Rename(UActorComponent& Component, const FString& Name);

	/**
	 * Convert an sRGB space float channels color, as used in AGX Dynamics' render materials, to a
	 * linear space float channels color, as used by Unreal Engine's render materials.
	 *
	 * @param SRGB An sRGB color with float channels in the 0..1 range.
	 * @return A linear color with float channels in the 0..1 range.
	 */
	static FLinearColor SRGBToLinear(const FVector4& SRGB);

	/**
	 * Convert a linear space float channels color, as used by Unreal Engine's render materials, to
	 * a sRGB space float channels color, as used by AGX Dynamics' render materials.
	 * @param Linear
	 * @return
	 */
	static FVector4 LinearToSRGB(const FLinearColor& Linear);
};
