#pragma once

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"

#include "Shapes/ShapeBarrier.h"

#include "AGX_SimpleMeshComponent.h"

#include "AGX_ShapeComponent.generated.h"


class UAGX_MaterialBase;


UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, NotPlaceable,
	Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_ShapeComponent : public UAGX_SimpleMeshComponent
{
	GENERATED_BODY()

public:

	/**
	 * Defines physical properties of both the surface and the bulk of this shape.
	 *
	 * Surface properties do for example greatly affect frictional forces.
	 *
	 * Bulk properties have impact on collision forces but also on Rigid Body mass.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	UAGX_MaterialBase* PhysicalMaterial;

	/**
	 * Toggle to enable or disable collision generation against this shape.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	bool bCanCollide = true;

	UAGX_ShapeComponent();

	virtual FShapeBarrier* GetNative() PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual const FShapeBarrier* GetNative() const PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual FShapeBarrier* GetOrCreateNative() PURE_VIRTUAL(UAGX_ShapeComponent::GetOrCreateNative, return nullptr;);
	bool HasNative() const;

	/**
	 * Re-creates (or destroys) the triangle mesh data for the visual representation
	 * of the shape to match the physical definition of the shape.
	 */
	void UpdateVisualMesh();

	/**
	 * Returns whether this shape needs have a visual mesh representation.
	 */
	bool ShouldCreateVisualMesh() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR

	/**
	 * Should be overridden by subclasses and return whether changing the
	 * value of the specified property will need an update of the visual mesh.
	 *
	 * For struct properties, MemberPropertyName is the struct member name and,
	 * PropertyName is the name of the specific member of the struct. For other
	 * properties, both names are the same.
	 *
	 * Subclass must invoke the Super class's implementation and use its result
	 * with a logical OR!
	 */
	virtual bool DoesPropertyAffectVisualMesh(const FName& PropertyName, const FName& MemberPropertyName) const;

	void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;

	virtual void PostLoad() override; // When loaded in Editor or Game

	virtual void PostInitProperties() override;

#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void ReleaseNative() PURE_VIRTUAL(UAGX_ShapeComponent::ReleaseNative,);

	/**
	 * Updates the local transform of the native geometry to match this component's
	 * transform relative to the actor. Must be called from each subclass immediately
	 * after initializing the native geometry.
	 */
	template<typename TNative>
	void UpdateNativeLocalTransform(TNative &Native);
	// TODO: Would be easier if Native was owned by ShapeComponent, with polymorphic pointer (e.g. TUniquePtr).

	/**
	 * Defines triangles for a visual mesh to render in Unreal Engine. Whether
	 * the mesh is always rendered or just for debug is for the user to decide.
	 * The mesh should be in local coordinates relative to this component,
	 * such that any inherited component transform (be aware of scale) that is
	 * applied after results in a rendered mesh that is correctly placed.
	 */
	virtual void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) {}//PURE_VIRTUAL(UAGX_ShapeComponent::CreateVisualMesh, );

private:
	// UAGX_ShapeComponent does not own the Barrier object because it cannot
	// name its type. It is instead owned by the typed subclass, such as
	// UAGX_BoxShapeComponent. Access to it is provided using virtual Get
	// functions.
};


template<typename TNative>
void UAGX_ShapeComponent::UpdateNativeLocalTransform(TNative& Native)
{
	FTransform RigiBodyTransform = GetOwner()->GetActorTransform();

	FVector LocalPosition = RigiBodyTransform.InverseTransformPositionNoScale(GetComponentLocation());
	FQuat LocalOrientation = RigiBodyTransform.InverseTransformRotation(GetComponentQuat());

	Native.SetLocalPosition(LocalPosition);
	Native.SetLocalRotation(LocalOrientation);
}
