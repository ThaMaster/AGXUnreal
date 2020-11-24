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
 * Defines the Rigid Body to be bound by a Constraint and an attachment frame that is
 * defined by the transform of either the Constraint itself, the Rigid Body or some other Actor
 * or Component plus an optional offset given by the Local Frame Location and Rotation.
 *
 * Whether the constraint itself, the Rigid Body or some other Actor or Component should be used
 * to define the attachment frame can be selected by changing the Frame Defining Source accordingly.
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
	TEnumAsByte<enum EAGX_FrameDefiningSource> FrameDefiningSource =
		EAGX_FrameDefiningSource::CONSTRAINT;

	/**
	 * The Frame Defining Component makes it possible to use the transform of any Actor or Component
	 * to define the attachment frame of the Constrained Rigid Body.
	 *
	 * Note that the two rigid bodies in a  constraint can use the same Frame Defining Component, or
	 * different, or one can have one and the other not.
	 * AGX Dynamics for Unreal provides Constraint Frame Component which is intended for this
	 * purpose. It provides constraint listing and visualization making it possible to see which
	 * constraints are using that Constraint Frame Component.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Frame Transformation",
		Meta = (EditCondition = "FrameDefiningSource == EAGX_FrameDefiningSource::OTHER"))
	FAGX_SceneComponentReference FrameDefiningComponent;

	/** Frame location relative to either the Constraint, the Rigid Body Actor or from the Frame
	 * Defining Actor. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FVector LocalFrameLocation;

	/** Frame rotation relative to to either the Constraint, the Rigid Body Actor or from the Frame
	 * Defining Actor. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FRotator LocalFrameRotation;

	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Calculates and returns the frame location given in the RigidBody's frame
	 * (or in world space if not set, which gives a warning).
	 */
	FVector GetLocalFrameLocationFromBody() const;

	/**
	 * Calculates and returns the frame rotation given in the RigidBody's frame
	 * (or in world space if not set, which gives a warning).
	 */
	FQuat GetLocalFrameRotationFromBody() const;

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

	// Returns the currently active FrameDefiningComponent given the FrameDefiningSource selected.
	USceneComponent* GetFinalFrameDefiningComponent() const;
};
