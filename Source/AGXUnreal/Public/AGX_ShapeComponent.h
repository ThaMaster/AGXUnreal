#pragma once

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"

#include "ShapeBarrier.h"


#include "AGX_ShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract, NotPlaceable,
	Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_ShapeComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ShapeComponent();

	virtual FShapeBarrier* GetNative() PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual const FShapeBarrier* GetNative() const PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);
	virtual FShapeBarrier* GetOrCreateNative() PURE_VIRTUAL(UAGX_ShapeComponent::GetOrCreateNative, return nullptr;);
	bool HasNative() const;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
	void UpdateNativeTransform(TNative &Native);
	// TODO: Would be easier if Native was owned by ShapeComponent, with polymorphic pointer (e.g. TUniquePtr).

private:
	// UAGX_ShapeComponent does not own the Barrier object because it cannot
	// name its type. It is instead owned by the typed subclass, such as
	// UAGX_BoxShapeComponent. Access to it is provided using virtual Get
	// functions.
};


template<typename TNative>
void UAGX_ShapeComponent::UpdateNativeTransform(TNative& Native)
{
	FTransform RigiBodyTransform = GetOwner()->GetActorTransform();

	FVector LocalPosition = RigiBodyTransform.InverseTransformPosition(GetComponentLocation());
	FQuat LocalOrientation = RigiBodyTransform.InverseTransformRotation(GetComponentQuat());

	Native.SetLocalPosition(LocalPosition, GetWorld());
	Native.SetLocalRotation(LocalOrientation);
}
