#pragma once

// AGXUnreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_SceneComponentReference.h"
#include "Constraints/AGX_ConstraintEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintBodyAttachment.generated.h"

class UAGX_ConstraintComponent;
class UAGX_RigidBodyComponent;
class AAGX_ConstraintFrameActor;
class FRigidBodyBarrier;

/**
 * Defines the Rigid Body to be bound by a Constraint and its Local Frame Location
 * and Rotation.
 *
 * The actual usage of the Local Frame Location and Rotation varies dependening on
 * constraint type, but it can generally be seen as the local points (on the rigid bodies)
 * that should in some way be glewed together by the constraint.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintBodyAttachment
{
	GENERATED_USTRUCT_BODY()

	FAGX_ConstraintBodyAttachment();
	FAGX_ConstraintBodyAttachment(USceneComponent* InOwner);

	/// \todo Cannot assume a single body per actor. Should we change the UPROPERTY
	/// to be a UAGX_RigidBodyComponent instead, or should we keep the Actor
	/// reference and also keep some kind of component identifier? Should we use
	/// FComponentRef here?

	UPROPERTY(EditAnywhere, Category = "Rigid Body")
	FAGX_RigidBodyReference RigidBody;

	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	TEnumAsByte<enum EAGX_FrameDefiningMode> FrameDefiningMode = EAGX_FrameDefiningMode::CONSTRAINT;

	/**
	 * Optional. Use this to define the Local Frame Location and Rotation relative to a Component
	 * other then the Rigid Body Component. This is used for convenience during setup only, the
	 * actual frame transforms used by the simulation will nevertheless be calculated and stored
	 * relative to the rigid body when the simulation starts.
	 *
	 * Not that the two rigid bodies in a  constraint can use the same Frame Defining Component, or
	 * different, or one can have one and the other not. It's even possible to use one of the
	 * constrainted bodies as the Frame Defining Component for both of them.
	 *
	 * AGX Dynamics for Unreal provides Constraint Frame Component which is intended for this
	 * purpose. It provides constraint listing and visualization making it possible to see which
	 * constraints are using that Constraint Frame Component.
	 */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation", Meta = (EditCondition = "bCanEditFrameDefiningComponent"))
	FAGX_SceneComponentReference FrameDefiningComponent;

	/** Frame location relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FVector LocalFrameLocation;

	/** Frame rotation relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FRotator LocalFrameRotation;

	UPROPERTY(EditAnywhere)
	bool bCanEditFrameDefiningComponent;

	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Calculates and returns the frame location relative to Rigid Body Actor
	 * (or in world space if not set). I.e. if Frame Defining Actor is set, the
	 * returned value is Local Frame Location transformed from Frame Defining Actor's
	 * to Rigid Body Actor's transform space, and if not set the returned value is
	 * just Local Frame Location.
	 */
	FVector GetLocalFrameLocation() const;

	/**
	 * Calculates and returns the frame rotation relative to Rigid Body Actor
	 * (or in world space if not set). I.e. if Frame Defining Actor is set, the
	 * returned value is Local Frame Rotation transformed from Frame Defining Actor's
	 * to Rigid Body Actor's transform space, and if not set the returned value is
	 * just Local Frame Rotation.
	 */
	FQuat GetLocalFrameRotation() const;

	/**
	 * Calculates and returns the frame location in world space.
	 */
	FVector GetGlobalFrameLocation() const;

	/**
	 * Calculates and returns the frame location in world space as-if this attachment was attached
	 * to the given RigidBody.
	 *
	 * As with the regular, parameter-less, GetGlobalFrameLocation, the RigidBody is ignored if a
	 * FrameDefiningActor has been set.
	 *
	 * This is used while in the Blueprint editor, where we don't yet have a "real"
	 * ConstraintBodyAttachment or a "real" Actor that owns it.
	 *
	 * @param Body The body that the local location should be relative too, if no
	 * FrameDefiningActor.
	 * @return The world location that this ConstraintBodyAttachment represents.
	 */
	FVector GetGlobalFrameLocation(UAGX_RigidBodyComponent* Body) const;

	/**
	 * Calculates and returns the frame rotation in world space.
	 */
	FQuat GetGlobalFrameRotation() const;

	/**
	 * Calculates and returns the frame rotation in world space as-if this attachment was attached
	 * to the given RigidBody.
	 *
	 * As with the regular, parameter-less, GetGlobalFrameRotation, the RigidBody is ignored if a
	 * FrameDefiningActor has been set.
	 *
	 * This is used while in the Blueprint editor, where we don't yet have a "real"
	 * ConstraintBodyAttachment or a "real" Actor that owns it.
	 *
	 * @param Body The body that the local rotation should be relative too, if no
	 * FrameDefiningActor.
	 * @return The world rotation that this ConstraintBodyAttachment represents.
	 */
	FQuat GetGlobalFrameRotation(UAGX_RigidBodyComponent* Body) const;

	FMatrix GetGlobalFrameMatrix() const;

	FMatrix GetGlobalFrameMatrix(UAGX_RigidBodyComponent* Body) const;

	FRigidBodyBarrier* GetRigidBodyBarrier(bool CreateIfNeeded);

	USceneComponent* Owner;

#if WITH_EDITOR
	/**
	 * Should be invoked whenever FrameDefiningComponent changes, to trigger the
	 * removal of the constraint from the previous FrameDefiningComponent's list of
	 * constraint usages, and adding to the new one's (if they are
	 * AAGX_ConstraintFrameActor actor types).
	 */
	void OnFrameDefiningComponentChanged(UAGX_ConstraintComponent* Parent);

	void OnDestroy(UAGX_ConstraintComponent* Parent);
#endif

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY(Transient)
	mutable AActor* RecentFrameDefiningActor;
	USceneComponent* PreviousFrameDefiningComponent;
};
