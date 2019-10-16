#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintStructs.generated.h"


class AAGX_Constraint;
class AAGX_ConstraintFrameActor;
class FRigidBodyBarrier;


/**
 * A struct for a property that has one double component per DOF (Degree of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintDoublePropertyPerDof
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

	FAGX_ConstraintDoublePropertyPerDof(double DefaultValue = 0.0, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		:
		Translational_1(DefaultValue),
		Translational_2(DefaultValue),
		Translational_3(DefaultValue),
		Rotational_1(DefaultValue),
		Rotational_2(DefaultValue),
		Rotational_3(DefaultValue),
		Translational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_1),
		Translational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_2),
		Translational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_3),
		Rotational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_1),
		Rotational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_2),
		Rotational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_3)
	{
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
 * A struct for a property that has one float range component per DOF (Degree of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangePropertyPerDof
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_1_IsEditable"))
	FFloatInterval Translational_1;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_2_IsEditable"))
	FFloatInterval Translational_2;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Translational_3_IsEditable"))
	FFloatInterval Translational_3;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_1_IsEditable"))
	FFloatInterval Rotational_1;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_2_IsEditable"))
	FFloatInterval Rotational_2;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "Rotational_3_IsEditable"))
	FFloatInterval Rotational_3;

	FAGX_ConstraintRangePropertyPerDof(float DefaultMinValue = 0.0f, float DefaultMaxValue = 0.0f, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		:
		Translational_1(DefaultMinValue, DefaultMaxValue),
		Translational_2(DefaultMinValue, DefaultMaxValue),
		Translational_3(DefaultMinValue, DefaultMaxValue),
		Rotational_1(DefaultMinValue, DefaultMaxValue),
		Rotational_2(DefaultMinValue, DefaultMaxValue),
		Rotational_3(DefaultMinValue, DefaultMaxValue),
		Translational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_1),
		Translational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_2),
		Translational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_TRANSLATIONAL_3),
		Rotational_1_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_1),
		Rotational_2_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_2),
		Rotational_3_IsEditable((uint8)EditableDofs & (uint8)EDofFlag::DOF_FLAG_ROTATIONAL_3)
	{
	}

	FFloatInterval operator[](int32 Index)const
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
	 * Note that both rigid bodies can use the same frame defining actor, or one rigid body
	 * can use the other rigid body as frame defining actor, etc.
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

	void OnDestroy(AAGX_Constraint* Owner);

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY(Transient)
	mutable AActor* RecentFrameDefiningActor;
#endif
};


/**
 * Range controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangeController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/** Range in Degrees if controller is on a Rotational Degree-Of-Freedom,  else in Centimeters. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval Range;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:

	FAGX_ConstraintRangeController(bool bRotational = false);

	void ToBarrier(struct FRangeControllerBarrier* Barrier) const;

private:

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};


/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintTargetSpeedController
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	bool bEnable;

	/**
	 * Target Speed in Degrees Per Second if controller is on a Rotational DOF,
	 * else in Centimeters Per Second.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Speed;

	/**
	 * Whether the controller should auto-lock whenever target speed is zero,
	 * such that it will not drift away from that angle/position if the assigned
	 * force range is enough to hold it.
	 */
	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	bool bLockedAtZeroSpeed;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Elasticity;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	double Damping;

	UPROPERTY(EditAnywhere, Meta = (EditCondition = "bEnable"))
	FFloatInterval ForceRange;

public:

	FAGX_ConstraintTargetSpeedController(bool bRotational = false);

	void ToBarrier(struct FTargetSpeedControllerBarrier* Barrier) const;

private:

	// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;
};
