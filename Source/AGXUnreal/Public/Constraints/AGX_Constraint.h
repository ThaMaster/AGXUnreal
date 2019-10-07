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
 * Constraint type independent index of Degree of Freedom (DOF). Does never change
 * index order layout, even in derived constraints, contrary to the AGX's native
 * constraint-specific DOF indexes.
 */
UENUM()
enum class EGenericDofIndex
{
	/** All degrees of freedom */
	ALL_DOF = -1			UMETA(DisplayName = "All"),

	/** DOF for the first translational axis */
	TRANSLATIONAL_1 = 0		UMETA(DisplayName = "Translation1"),

	/** DOF for the second translational axis */
	TRANSLATIONAL_2 = 1		UMETA(DisplayName = "Translation2"),

	/** DOF for the third translational axis */
	TRANSLATIONAL_3 = 2		UMETA(DisplayName = "Translation3"),

	/** DOF corresponding to the first rotational axis */
	ROTATIONAL_1 = 3		UMETA(DisplayName = "Rotation1"),

	/** DOF corresponding to the second rotational axis */
	ROTATIONAL_2 = 4		UMETA(DisplayName = "Rotation2"),

	/** DOF for rotation around Z-axis */
	ROTATIONAL_3 = 5		UMETA(DisplayName = "Rotation3"),
};


/**
 * Flags used to be able to identify DOFs and combine them into a bitmask.
 */
UENUM(meta = (Bitflags))
enum class EDofFlag : uint8
{
	DOF_FLAG_ALL = 0x3F						UMETA(DisplayName = "All"),
	DOF_FLAG_TRANSLATIONAL_1 = 1 << 0		UMETA(DisplayName = "Translation1"),
	DOF_FLAG_TRANSLATIONAL_2 = 1 << 1		UMETA(DisplayName = "Translation2"),
	DOF_FLAG_TRANSLATIONAL_3 = 1 << 2		UMETA(DisplayName = "Translation3"),
	DOF_FLAG_ROTATIONAL_1 = 1 << 3			UMETA(DisplayName = "Rotation1"),
	DOF_FLAG_ROTATIONAL_2 = 1 << 4			UMETA(DisplayName = "Rotation2"),
	DOF_FLAG_ROTATIONAL_3 = 1 << 5			UMETA(DisplayName = "Rotation3"),
};


/**
 * A struct for a property that has one value component per DOF (Degree of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintDofProperty
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_1_IsEditable"))
	double Translational_1;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_2_IsEditable"))
	double Translational_2;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_3_IsEditable"))
	double Translational_3;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_1_IsEditable"))
	double Rotational_1;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_2_IsEditable"))
	double Rotational_2;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_3_IsEditable"))
	double Rotational_3;

	FAGX_ConstraintDofProperty(double defaultValue = 0.0, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		:
		Translational_1(defaultValue),
		Translational_2(defaultValue),
		Translational_3(defaultValue),
		Rotational_1(defaultValue),
		Rotational_2(defaultValue),
		Rotational_3(defaultValue),
		Translational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_1),
		Translational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_2),
		Translational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_3),
		Rotational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_1),
		Rotational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_2),
		Rotational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_3)
	{ 
		UE_LOG(LogTemp, Log, TEXT("EditableDofs = %d"), (uint8)EditableDofs);
	}

	double operator[](int32 Index)const
	{
		check(Index >= 0 && Index < 6);
		return (&Translational_1)[Index];
	}


private:

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Translational_1_IsEditable;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Translational_2_IsEditable;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Translational_3_IsEditable;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Rotational_1_IsEditable;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Rotational_2_IsEditable;

	UPROPERTY(Transient, VisibleDefaultsOnly)
	bool Rotational_3_IsEditable;

};


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
 * Each non-abstract subclass must:
 *   1. Implement CreateNativeImpl (see method comment).
 *   2. In the constructor pass the constraint type specific array of locked DOFs to the
 *       overloaded AAGX_Constraint constructor. The array items and their indexes must exactly
 *       match the enum in the header of the native AGX constraint (without ALL_DOF and NUM_DOF).
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
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies", Meta=(EditCondition = "hej"))
	FAGX_ConstraintBodyAttachment BodyAttachment1;

	/**
	 * The second Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * If second Rigid Body is null, the first Rigid Body will be constrained to the World.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies")
	FAGX_ConstraintBodyAttachment BodyAttachment2;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDofProperty Elasticity;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDofProperty Damping;

public:

	AAGX_Constraint() { }

	AAGX_Constraint(const TArray<EDofFlag> &LockedDofsOrdered);

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

	void UpdateNativeProperties();

protected:

	virtual void BeginPlay() override;

	bool ToNativeDof(EGenericDofIndex GenericDof, int32 &NativeDof);

	/** Must be overriden by derived class, and create a NativeBarrier with allocated native. Nothing more.*/
	virtual void CreateNativeImpl() PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl,);

	TUniquePtr<FConstraintBarrier> NativeBarrier;

	// The Degrees of Freedom (DOF) that are locked by the specific constraint type,
	// ordered the way they are indexed by in the native AGX api (except for ALL_DOF and NUM_DOF).
	const TArray<EDofFlag> LockedDofs;

	// Mapping from EGenericDofIndex to native AGX constraint specific DOF index.
	// This list can change with each constraint type, and should exactly reflect
	// the DOF enum in the native header for each constraint.
	const TMap<EGenericDofIndex, int32> NativeDofIndexMap;

private:

	/** Invokes CreateNativeImpl, then adds the native to the simulation. */
	void CreateNative();
};
