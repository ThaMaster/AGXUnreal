// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Constraints/ConstraintBarrier.h" // TODO: Shouldn't be necessary here!

#include "AGX_Constraint.generated.h"

class AAGX_ConstraintFrameActor;
class FConstraintBarrier;
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

	/**
	 * The Actor containing the Rigid Body Component to be bound by the constraint.
	 */
	UPROPERTY(EditAnywhere, Category = "Rigid Body")
	AActor* RigidBodyActor;

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
	 * If used, it is recommended to use an AGX Constraint Frame Actor.
	 */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	AActor* FrameDefiningActor;
			
	/** Frame location relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FVector LocalFrameLocation;
	
	/** Frame rotation relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FRotator LocalFrameRotation;
	
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

	FRigidBodyBarrier* GetRigidBodyBarrier(bool CreateIfNeeded);

#if WITH_EDITOR

	/**
	 * Should be invoked whenever Frame Defining Actor changes, to trigger the removal
	 * of the constraint from the previous Frame Defining Actor's list of constraint usages,
	 * and adding to the new one's (if they are AAGX_ConstraintFrameActor actor types).
	 */
	void OnFrameDefiningActorChanged(AAGX_Constraint* Owner);

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY()
		mutable AActor* RecentFrameDefiningActor;
#endif

};


/**
 * Abstract base class for all AGX constraint types.
 *
 * Does not have a its own world space transform, but has references to the two
 * Rigid Body Actors to constrain to each other, and their attachment frames
 * (which defines how the rigid bodies should locally be jointed to one another).
 * 
 * At least the first Rigid Body Actor must be chosen. If there is no second Rigid Body Actor,
 * then the first one will be constrained to the static World instead.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract,
	meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_Constraint : public AActor
{
	GENERATED_BODY()

public:

	/**
	 * The first Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * Rigid Body Actor must be set.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	FAGX_ConstraintBodyAttachment BodyAttachment1;

	/**
	 * The second Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * If second Rigid Body is null, the first Rigid Body will be constrained to the World.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	FAGX_ConstraintBodyAttachment BodyAttachment2;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	double Compliance;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	double Damping;

public:

	AAGX_Constraint();
	virtual ~AAGX_Constraint();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override { return false; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Get the native AGX Dynamics representation of this constraint. Create it if necessary. */
	FConstraintBarrier* GetOrCreateNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	FConstraintBarrier* GetNative();

	/** Return true if the AGX Dynamics object has been created. False otherwise. */
	bool HasNative() const;

protected:

	virtual void BeginPlay() override;

	/** Must be overriden by derived class, and create a NativeBarrier with allocated native. Nothing more.*/
	virtual void CreateNativeImpl() PURE_VIRTUAL(AAGX_Constraint::CreateNative,);

	TUniquePtr<FConstraintBarrier> NativeBarrier;

private:

	/** Invokes CreateNativeImpl, then adds the native to the simulation. */
	void CreateNative();
};
