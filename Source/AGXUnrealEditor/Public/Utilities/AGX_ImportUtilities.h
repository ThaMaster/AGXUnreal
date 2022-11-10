// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Color.h"

class FTrimeshShapeBarrier;
class FRenderDataBarrier;
class FShapeBarrier;
class FMergeSplitThresholdsBarrier;
class FShapeMaterialBarrier;
class FTrackBarrier;
class FTrackPropertiesBarrier;
class FContactMaterialBarrier;
class UAGX_ContactMaterial;
class UAGX_MergeSplitThresholdsBase;
class UAGX_ShapeMaterial;
class UAGX_TrackInternalMergeProperties;
class UAGX_TrackProperties;
struct FAGX_RenderMaterial;

class AActor;
class FString;
class UActorComponent;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UStaticMesh;

class AGXUNREALEDITOR_API FAGX_ImportUtilities
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
	 * @param FallbackName Name to use if NativeName is unusable for some reason, for example is
	 * empty.
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
	 * Sets up the imported Trimesh as an UStaticMesh asset, but does not write it to disk.
	 *
	 * @param Trimesh The imported trimesh to be saved.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @param FallbackName Name to give the asset in case the trimesh doesn't have a source
	 * name.
	 * @return The created asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryName,
		const FString& FallbackName);

	/**
	 * Sets up the imported Render Data Mesh as an UStaticMesh asset, but does not write it to disk.
	 *
	 * @param RenderData The Render Data holding the render mesh to store.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @return The created asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FRenderDataBarrier& RenderData, const FString& DirectoryName);

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

	/**
	 * Store an imported AGX Dynamics Track Internal Merge Property as an
	 * UAGX_TrackInternalMergeProperties asset on drive..
	 * @param Barrier The imported Track owning the Internal Merge Property.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @param Name The name to give to the new asset. A sequence number will be added in case of a
	 * conflict.
	 * @return The created UAGX_TrackInternalMergeProperties asset.
	 */
	static UAGX_TrackInternalMergeProperties* SaveImportedTrackInternalMergePropertiesAsset(
		const FTrackBarrier& Barrier, const FString& DirectoryName, const FString& Name);

	/**
	 * Store an imported AGX Dynamics Track Property as an UAGX_TrackProperties.
	 * @param Barrier The imported Track referencing the Track Property.
	 * @param DirectoryName The name of the directory where the assets are collected.
	 * @param Name The name to give to the new asset. A sequence number will be added in case of a
	 * conflict.
	 * @return The created UAGX_TrackProperties.
	 */
	static UAGX_TrackProperties* SaveImportedTrackPropertiesAsset(
		const FTrackPropertiesBarrier& Barrier, const FString& DirectoryName, const FString& Name);

	/**
	 * Generate valid name for the object. Generates a fallback name if the given name can't be
	 * used.
	 */
	static FString CreateName(UObject& Object, const FString& Name);

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

	static FString GetImportRootDirectoryName();
	static FString GetImportShapeMaterialDirectoryName();
	static FString GetImportContactMaterialDirectoryName();
	static FString GetImportRenderMaterialDirectoryName();
	static FString GetImportMergeSplitThresholdsDirectoryName();
	static FString GetImportStaticMeshDirectoryName();
	static FString GetImportRenderMeshDirectoryName();

	static FString GetUnsetUniqueImportName();

	/**
	 * Create a new asset destined for the import directory. This functions will only create the
	 * asset and setup it's Package, it will not actually save it to disk. That is the
	 * responsibility of the caller.
	 */
	template <typename UAsset>
	static UAsset* CreateAsset(
		const FString& DirectoryName, FString AssetName, const FString& AssetType);

	/**
	 * Create a new Component and add it to an Actor and attach it to the given attach parent.
	 * The Component will be given a temporary unique name.
	 */
	template <typename TComponent>
	static TComponent* CreateComponent(AActor& Owner, USceneComponent& AttachParent);
};

template <typename UAsset>
UAsset* FAGX_ImportUtilities::CreateAsset(
	const FString& DirectoryName, FString AssetName, const FString& AssetType)
{
	AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, "", AssetType);
	FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(DirectoryName, AssetType);
	FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
	UPackage* Package = CreatePackage(*PackagePath);

	UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not create asset '%s' from '%s'."), *AssetName,
			*DirectoryName);
	}

	return Asset;
}

template <typename TComponent>
TComponent* FAGX_ImportUtilities::CreateComponent(AActor& Owner, USceneComponent& AttachParent)
{
	TComponent* Component = NewObject<TComponent>(
		&AttachParent, FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));

	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->AttachToComponent(
		&AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	return Component;
}
