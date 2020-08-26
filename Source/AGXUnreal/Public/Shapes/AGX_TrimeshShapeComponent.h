#pragma once

// AGXUnreal includes
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/TrimeshShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#include "AGX_TrimeshShapeComponent.generated.h"

/** Specifies from where to get the triangle data. */
UENUM()
enum EAGX_TrimeshSourceLocation
{
	/** Static Mesh from the first child component that is a Static Mesh Component. */
	TSL_CHILD_STATIC_MESH_COMPONENT UMETA(DisplayName = "Child Component"),

	/** Static Mesh from the first ancestor that is a Static Mesh Component. */
	TSL_PARENT_STATIC_MESH_COMPONENT UMETA(DisplayName = "Parent Component"),

	/** Directly from explicitly chosen Static Mesh Asset. */
	TSL_STATIC_MESH_ASSET UMETA(DisplayName = "Asset")
};

/**
 * Uses triangle data from a Static Mesh to generate an AGX Triangle Collision Mesh.
 *
 * The Static Mesh source can be either the parent or child Static Mesh Component,
 * or a specific Static Mesh Asset.
 */
UCLASS(
	ClassGroup = "AGX_Shape", Category = "AGX", Placeable, meta = (BlueprintSpawnableComponent),
	HideCategories = (HLOD, Lighting, LOD, Materials, MaterialParameters, Rendering))
class AGXUNREAL_API UAGX_TrimeshShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_TrimeshShapeComponent();

	/**
	 * Specifies from where should the Static Mesh triangle data be read.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	TEnumAsByte<EAGX_TrimeshSourceLocation> MeshSourceLocation;

	/**
	 * Only used if Mesh Source Location is set to Static Mesh Asset. Specifies
	 * which Static Mesh Asset to use.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	UStaticMesh* MeshSourceAsset;

	/**
	 * Whether to explicitly set LOD Level to read triangle data from here
	 * or to use the setting that already exists on the Static Mesh source.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape", AdvancedDisplay)
	bool bOverrideMeshSourceLodIndex;

	/**
	 * Only used if Override Mesh Source LOD Index is enabled. Specifies which LOD Level
	 * of the Static Mesh source to read triangle data from. Zero is the most detailed level.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shape", AdvancedDisplay,
		Meta = (EditCondition = "bOverrideMeshSourceLodIndex"))
	uint32 MeshSourceLodIndex;

	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;

	/// Get the native AGX Dynamics representation of this Trimesh. May return nullptr.
	FTrimeshShapeBarrier* GetNativeTrimesh();

	virtual void UpdateNativeProperties() override;

	/**
	 * Copy properties from the given AGX Dynamics trimesh into this component.
	 * Does not copy assets, so not triangle data and not material.
	 * Does copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics trimesh to copy from.
	 */
	void CopyFrom(const FTrimeshShapeBarrier& Barrier);

protected:
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;

#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;

	virtual bool CanEditChange(
#if UE_VERSION_OLDER_THAN(4,25,0)
		const UProperty* InProperty
#else
		const FProperty* InProperty
#endif
		) const override;
#endif

private:
	/// Create the AGX Dynamics objects owned by the FTrimeshShapeBarrier.
	void CreateNative();

	// Tell the Barrier object to release its references to the AGX Dynamics objects.
	virtual void ReleaseNative() override;

	// BeginPlay/EndPlay is handled by the base class UAGX_ShapeComponent.

	bool FindStaticMeshSource(UStaticMesh*& StaticMesh, FTransform* WorldTransform) const;

	/**
	 * Uses data from the Static Mesh source asset to construct a simplified
	 * vertex and index buffer. The simplification is mainly due to the fact that
	 * the source render mesh might need multiple vertices with same position but
	 * different normals, texture coordinates, etc, while the collision mesh can
	 * share vertices between triangles more aggresively.
	 */
	bool GetStaticMeshCollisionData(TArray<FVector>& Vertices, TArray<FTriIndices>& Indices) const;

private:
	FTrimeshShapeBarrier NativeBarrier;
};
