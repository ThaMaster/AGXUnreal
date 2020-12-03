#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"
#include "BoxShapeBarrier.h"
#include "SphereShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"

#include "AGX_StaticMeshComponent.generated.h"

USTRUCT(BlueprintType)
struct FAGX_Shape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	bool bCanCollide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	class UAGX_ShapeMaterialBase* Material = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	TArray<FName> CollisionGroups;

// This was supposed to allow us to map physics shape settings to the correct
// collision shape after changing the collision shape setup but I don't think
// it will work because the collision shapes are stored by-value in TArrays so
// adding or removing collision shapes will cause this pointer to become invalid.
//
// Not sure what else to do. For now I'm just doing 1:1 on the indices. The first
// physics sphere apply to the first collision sphere and so on.
#if 0
	FKShapeElem* CollisionShape;
#endif
};

#if 0
USTRUCT(BlueprintType)
struct FAGX_Sphere : public FAGX_Shape
{
	GENERATED_BODY()

	FAGX_Sphere()
		: Radius(0.5f)
	{
	}

	FAGX_Sphere(float InRadius)
		: Radius(InRadius)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	float Radius;
};

USTRUCT(BlueprintType)
struct FAGX_Box : public FAGX_Shape
{
	GENERATED_BODY()

	FAGX_Box()
		: HalfExtent(0.5f)
	{
	}

	FAGX_Box(FVector InHalfExtent)
		: HalfExtent(InHalfExtent)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	FVector HalfExtent;
};
#endif

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = ("Physics"))
class AGXUNREAL_API UAGX_StaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public: // Properties.
	UAGX_StaticMeshComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	FAGX_Shape DefaultShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	TArray<FAGX_Shape> Spheres;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	TArray<FAGX_Shape> Boxes;

public: // Public member functions.
	bool HasNative() const;
	FRigidBodyBarrier* GetNative();
	const FRigidBodyBarrier* GetNative() const;
	FRigidBodyBarrier* GetOrCreateNative();

	/// Bound to the source mesh asset's OnMeshChanged event/delegate.
	void OnMeshChanged();

	// Resize the shape data arrays to match the size of the collision shapes in the StaticMesh
	// asset.
	void RefreshCollisionShapes();

public: // Inherited interfaces.
	//~ Begin public UActorComponent interface.

	virtual void BeginPlay();

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	//~ End public UActorComponent interface.

protected: // Inherited interfaces.
	//~ Begin protected UActorComponent interface.
	virtual bool ShouldCreatePhysicsState() const override;
	virtual void OnCreatePhysicsState() override;
	//~ End protected UActorComponent interface.

#if WITH_EDITOR
	//~ Begin UObject interface.
	virtual void PreEditChange(FProperty* Property) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	//~ End UObject interface.
#endif

private: // Private member functions.
	void UpdateCollisionShapes(UStaticMeshComponent* Self);
	void AllocateNative();

	void ReadTransformFromNative();
	void WriteTransformToNative();

private: // Private member variables.
	FRigidBodyBarrier NativeBarrier;
	TArray<FSphereShapeBarrier> SphereBarriers;
	TArray<FBoxShapeBarrier> BoxBarriers;

	FDelegateHandle MeshChangedHandle;
};
