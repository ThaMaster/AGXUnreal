#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"
#include "BoxShapeBarrier.h"
#include "SphereShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"

#include "AGX_StaticMeshComponent.generated.h"

/**
 * AGX Dynamics data that we store per shape.
 */
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

// This was supposed to allow us to map AGX shape settings to the correct
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

/**
 * A StaticMeshComponent that uses AGX Dynamics for physics simulation instead of the built-in
 * physics engine. Very much experimental and work-in-progress.
 *
 * Just like the built-in StaticMeshComponent, UAGX_StaticMeshComponent references a StaticMesh
 * assets and reads the simulation setup, e.g., collision shapes, from that. The StaticMesh assets
 * contains some properties that are unrepresentable in AGX Dynamics, and AGX Dynamics has some
 * properties that aren't part of the StaticMesh asset so the conversion is performed on a
 * best-effort basis. Some AGX Dynamics specific per-shape properties are stored in FAGX_Shape
 * instances, one per collision shape in the StaticMesh asset.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = ("Physics", "Collision"))
class AGXUNREAL_API UAGX_StaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public: // Properties.
	UAGX_StaticMeshComponent();

	/**
	 * When a new collision shape is discovered the AGX Dynamics properties, i.e., the FAGX_Shape
	 * instance for that collision shape, is initialized from this template. It is not used directly
	 * during simulation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	FAGX_Shape DefaultShape;

	/**
	 * AGX Dynamics properties for each collision sphere.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	TArray<FAGX_Shape> Spheres;

	/**
	 * AGX Dynamics properties for each collision box.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics|Shapes")
	TArray<FAGX_Shape> Boxes;

public: // Public member functions.
	bool HasNative() const;
	FRigidBodyBarrier* GetNative();
	const FRigidBodyBarrier* GetNative() const;
	FRigidBodyBarrier* GetOrCreateNative();

	/// Bound to the source mesh asset's OnMeshChanged event/delegate.
	void OnMeshChanged();

	/**
	 * Resize the shape data arrays to match the size of the collision shapes in the StaticMesh
	 * asset. Any new FAGX_Shape instances created are initialized according to DefaultShape.
	 */
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

	//~ Begin UObject interface.
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* Property) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	//~ End UObject interface.

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
