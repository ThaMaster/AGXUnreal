#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_AutoFitShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", NotPlaceable)
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
	UPROPERTY(EditAnywhere, Category = "AGX Shape Auto-fit")
	UStaticMesh* MeshSourceAsset;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	void AutoFitToMesh();

	virtual void AutoFit(const TArray<FVector>& Vertices)
		PURE_VIRTUAL(UAGX_AutoFitShapeComponent::AutoFit, return;);

private:
	bool GetStaticMeshCollisionData(
		TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices) const;
};
