#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MeshWithTransform.h"
#include "Shapes/AGX_ShapeComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_AutoFitShapeComponent.generated.h"

UCLASS(
	ClassGroup = "AGX_Shape", Category = "AGX", NotPlaceable, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_AutoFitShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	/**
	 * Specifies from where should the Static Mesh triangle data be read.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Auto-fit")
	TEnumAsByte<EAGX_StaticMeshSourceLocation> MeshSourceLocation;

	/**
	 * Only used if Mesh Source Location is set to Static Mesh Asset. Specifies
	 * which Static Mesh Asset to use.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shape Auto-fit",
		Meta =
			(EditCondition =
				 "MeshSourceLocation == EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET"))
	UStaticMesh* MeshSourceAsset;

	/*
	 * Auto-fits to the collection of FAGX_MeshWithTransforms.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	bool AutoFit(TArray<FAGX_MeshWithTransform> Meshes);

	/*
	 * Auto-fits to whichever Static Mesh(es) are pointed out by MeshSourceLocation.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	bool AutoFitFromSelection();

	virtual bool AutoFitFromVertices(const TArray<FVector>& Vertices)
		PURE_VIRTUAL(UAGX_AutoFitShapeComponent::AutoFit, return false;);

private:
	bool AutoFitToChildrenFromSelection();

	TArray<FAGX_MeshWithTransform> GetSelectedStaticMeshes() const;
};
