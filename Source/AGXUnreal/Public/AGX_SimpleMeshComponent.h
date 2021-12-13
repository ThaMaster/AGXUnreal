// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "AGX_SimpleMeshComponent.generated.h"

class FPrimitiveSceneProxy;

struct AGXUNREAL_API FAGX_SimpleMeshData
{
	TArray<FVector> Vertices; // mandatory
	TArray<FVector2D> TexCoords;
	TArray<FVector> Normals; // mandatory
	TArray<FVector> Tangents;
	TArray<uint32> Indices;
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(
	hidecategories = (Object, LOD, Physics, Collision), NotPlaceable, editinlinenew,
	meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class AGXUNREAL_API UAGX_SimpleMeshComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

	/**
	 * Set the geometry to use on this triangle mesh.
	 * Vertex positions and normals are mandatory!
	 */
	bool SetMeshData(const TSharedPtr<FAGX_SimpleMeshData>& Data);

	/** Removes all geometry from this triangle mesh. */
	void ClearMeshData();

private:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	TSharedPtr<FAGX_SimpleMeshData> MeshData;

	friend class FAGX_SimpleMeshSceneProxy;
};
