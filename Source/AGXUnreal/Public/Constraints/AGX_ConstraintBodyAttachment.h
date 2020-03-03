#pragma once

// AGXUnreal includes.
#include "AGX_RigidBodyReference.h"

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

	/// \todo Cannot assume a single body per actor. Should we change the UPROPERTY
	/// to be a UAGX_RigidBodyComponent instead, or should we keep the Actor
	/// reference and also keep some kind of component identifier? Should we use
	/// FComponentRef here?

	UPROPERTY(EditAnywhere, Category = "Rigid Body")
	FAGX_RigidBodyReference RigidBody;

	/**
	 * Optional. Use this to define the Local Frame Location and Rotation relative to
	 * an actor other than the Rigid Body Actor (or to use the other Actor's transform
	 * directly by setting Local Frame Location and Rotation to zero). It is recommended
	 * to use the dedicated AGX Constraint Frame Actor, but any other actor can also be used.
	 *
	 * This is used for convenience only. The actual local frame transform used by
	 * the simulation will nevertheless be calculated and stored relative to the
	 * rigid body when the simulation is starting.
	 *
	 * Note that both rigid bodies can use the same frame defining actor, or one rigid body
	 * can use the other rigid body as frame defining actor, etc.
	 */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	AActor* FrameDefiningActor;

	/** Frame location relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FVector LocalFrameLocation;

	/** Frame rotation relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FRotator LocalFrameRotation;

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
	 * Calculates and returns the frame rotation in world space.
	 */
	FQuat GetGlobalFrameRotation() const;

	FMatrix GetGlobalFrameMatrix() const;

	FRigidBodyBarrier* GetRigidBodyBarrier(bool CreateIfNeeded);

#if WITH_EDITOR
	/**
	 * Should be invoked whenever Frame Defining Actor changes, to trigger the
	 * removal of the constraint from the previous Frame Defining Actor's list of
	 * constraint usages, and adding to the new one's (if they are
	 * AAGX_ConstraintFrameActor actor types).
	 */
	void OnFrameDefiningActorChanged(UAGX_ConstraintComponent* Parent);

	/**
	 * Should be called whenever the RigidBodyReference is changed.
	 *
	 * FAGX_ConstraintBodyAttachment doesn't inherit from anything, so it cannot
	 * have virtual functions for property changed callbacks. Instead the owning
	 * UAGX_ConstraintComponent must listen for the events and call here.
	 */
	void OnRigidBodyReferenceChanged();

	void OnDestroy(UAGX_ConstraintComponent* Parent);
#endif

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY(Transient)
	mutable AActor* RecentFrameDefiningActor;
};
