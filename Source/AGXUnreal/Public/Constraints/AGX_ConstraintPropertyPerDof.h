#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintPropertyPerDof.generated.h"

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
		: Translational_1(DefaultValue)
		, Translational_2(DefaultValue)
		, Translational_3(DefaultValue)
		, Rotational_1(DefaultValue)
		, Rotational_2(DefaultValue)
		, Rotational_3(DefaultValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_3)
	{
	}

	double operator[](int32 Index) const
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

	FAGX_ConstraintRangePropertyPerDof(
		float DefaultMinValue = 0.0f, float DefaultMaxValue = 0.0f, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: Translational_1(DefaultMinValue, DefaultMaxValue)
		, Translational_2(DefaultMinValue, DefaultMaxValue)
		, Translational_3(DefaultMinValue, DefaultMaxValue)
		, Rotational_1(DefaultMinValue, DefaultMaxValue)
		, Rotational_2(DefaultMinValue, DefaultMaxValue)
		, Rotational_3(DefaultMinValue, DefaultMaxValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_TRANSLATIONAL_3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DOF_FLAG_ROTATIONAL_3)
	{
	}

	FFloatInterval operator[](int32 Index) const
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
