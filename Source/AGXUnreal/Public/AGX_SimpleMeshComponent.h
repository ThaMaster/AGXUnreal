// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "AGX_SimpleMeshComponent.generated.h"

class FPrimitiveSceneProxy;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SimpleMeshTriangle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Triangle)
	FVector Vertex0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Triangle)
	FVector Vertex1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Triangle)
	FVector Vertex2;
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(hidecategories=(Object,LOD, Physics, Collision), editinlinenew, meta=(BlueprintSpawnableComponent), ClassGroup=Rendering)
class AGXUNREAL_API UAGX_SimpleMeshComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

	/** Set the geometry to use on this triangle mesh */
	UFUNCTION(BlueprintCallable, Category="Components|CustomMesh")
	bool SetMeshTriangles(const TArray<FAGX_SimpleMeshTriangle>& Triangles);

	/** Add to the geometry to use on this triangle mesh.  This may cause an allocation.  Use SetMeshTriangles() instead when possible to reduce allocations. */
	UFUNCTION(BlueprintCallable, Category = "Components|CustomMesh")
	void AddMeshTriangles(const TArray<FAGX_SimpleMeshTriangle>& Triangles);

	/** Removes all geometry from this triangle mesh.  Does not deallocate memory, allowing new geometry to reuse the existing allocation. */
	UFUNCTION(BlueprintCallable, Category = "Components|CustomMesh")
	void ClearMeshTriangles();

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

	/** */
	TArray<FAGX_SimpleMeshTriangle> MeshTris;

	friend class FAGX_SimpleMeshSceneProxy;
};


