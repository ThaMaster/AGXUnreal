#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"
#include "AGX_SimpleMeshComponent.h"
#include "Shapes/Contacts/AGX_ShapeContact.h"
#include "Shapes/AGX_ShapeEnums.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"

#include "AGX_ShapeComponent.generated.h"

class UMaterial;
class UAGX_ShapeMaterialBase;

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Abstract, NotPlaceable,
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	UAGX_ShapeMaterialBase* PhysicalMaterial;

	/**
	 * Toggle to enable or disable collision generation against this shape.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	bool bCanCollide = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetCanCollide(bool CanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	bool GetCanCollide() const;

	/**
	 * List of collision groups that this shape component is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	TArray<FName> CollisionGroups;

	/**
	 * Determines whether this shape should act as a sensor.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts")
	bool bIsSensor;

	/**
	 * Determines the sensor type. Only relevant if the Is Sensor property is checked.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts", Meta = (EditCondition = "bIsSensor"))
	TEnumAsByte<enum EAGX_ShapeSensorType> SensorType = EAGX_ShapeSensorType::ContactsSensor;

	/**
	 * Enable or disable this shape as a sensor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	void SetIsSensor(bool IsSensor);

	/**
	 * Returns true if this shape is a sensor, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	bool GetIsSensor() const;

	/**
	 * Get all shape contacts. Important: The data returned is only valid during a single simulation
	 * step. This function must therefore be called each time that the contact data is accessed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	TArray<FAGX_ShapeContact> GetShapeContacts();

	UAGX_ShapeComponent();

	virtual FShapeBarrier* GetNative()
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual const FShapeBarrier* GetNative() const
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual FShapeBarrier* GetOrCreateNative()
		PURE_VIRTUAL(UAGX_ShapeComponent::GetOrCreateNative, return nullptr;);
	bool HasNative() const;

	/**
	 * Re-creates (or destroys) the triangle mesh data for the visual representation
	 * of the shape to match the physical definition of the shape.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void UpdateVisualMesh();

	/**
	 * Returns whether this shape needs have a visual mesh representation.
	 */
	bool ShouldCreateVisualMesh() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

	void AddCollisionGroup(const FName& GroupName);

	void RemoveCollisionGroupIfExists(const FName& GroupName);

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
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostLoad() override; // When loaded in Editor or Game

	virtual void PostInitProperties() override;

	virtual void OnComponentCreated() override;

#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void ReleaseNative() PURE_VIRTUAL(UAGX_ShapeComponent::ReleaseNative, );

	/**
	 * Copy properties from the given AGX Dynamics shape into this component.
	 * Does not copy referenced attributes such as material properties.
	 * Called from each subclass' type-specific CopyFrom.
	 * @param Barrier The AGX Dynamics shape to copy from.
	 */
	void CopyFrom(const FShapeBarrier& Barrier);

	/**
	 * Updates the local transform of the native geometry to match this component's
	 * transform relative to the actor. Must be called from each subclass immediately
	 * after initializing the native geometry.
	 */
	template <typename TNative>
	void UpdateNativeLocalTransform(TNative& Native);
	// TODO: Would be easier if Native was owned by ShapeComponent, with polymorphic pointer (e.g.
	// TUniquePtr).

	/**
	 * Write the global/world Unreal transform to the local/relative transform of the native shape.
	 * This should be used only for body-less stand-alone shapes for which the native parent frame
	 * is the world coordinate system.
	 */
	void UpdateNativeGlobalTransform();

	/**
	 * Defines triangles for a visual mesh to render in Unreal Engine. Whether
	 * the mesh is always rendered or just for debug is for the user to decide.
	 * The mesh should be in local coordinates relative to this component,
	 * such that any inherited component transform (be aware of scale) that is
	 * applied after results in a rendered mesh that is correctly placed.
	 */
	virtual void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
	{
	} // PURE_VIRTUAL(UAGX_ShapeComponent::CreateVisualMesh, );

private:
	void ApplySensorMaterial();
	void RemoveSensorMaterial();

	// UAGX_ShapeComponent does not own the Barrier object because it cannot
	// name its type. It is instead owned by the typed subclass, such as
	// UAGX_BoxShapeComponent. Access to it is provided using virtual Get
	// functions.
};

template <typename TNative>
void UAGX_ShapeComponent::UpdateNativeLocalTransform(TNative& Native)
{
	// This assumes that the Shape's parent is the UAGX_RigidBody. If we want
	// to support other/deeper hierarchies then we need to find the total
	// transformation from the UAGX_RigidBody to this shape.
	Native.SetLocalPosition(GetRelativeTransform().GetLocation());
	Native.SetLocalRotation(GetRelativeTransform().GetRotation());
}
