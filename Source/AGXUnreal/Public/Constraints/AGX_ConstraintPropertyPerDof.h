// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintEnums.h"

// Unreal Engine includes.
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
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_1_IsEditable"))
	FAGX_Real Translational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_2_IsEditable"))
	FAGX_Real Translational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_3_IsEditable"))
	FAGX_Real Translational_3;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_1_IsEditable"))
	FAGX_Real Rotational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_2_IsEditable"))
	FAGX_Real Rotational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_3_IsEditable"))
	FAGX_Real Rotational_3;

	FAGX_ConstraintDoublePropertyPerDof(
		double DefaultValue = 0.0, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: Translational_1(DefaultValue)
		, Translational_2(DefaultValue)
		, Translational_3(DefaultValue)
		, Rotational_1(DefaultValue)
		, Rotational_2(DefaultValue)
		, Rotational_3(DefaultValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational3)
	{
	}

	double operator[](int32 Index) const
	{
		check(Index >= 0 && Index < NumGenericDofs);
		return (&Translational_1)[Index];
	}

	double operator[](EGenericDofIndex Index) const
	{
		return operator[](static_cast<int32>(Index));
	}

	double& operator[](int32 Index)
	{
		check(Index >= 0 && Index < NumGenericDofs);
		return (&Translational_1)[Index];
	}

	double& operator[](EGenericDofIndex Index)
	{
		return operator[](static_cast<int32>(Index));
	}

private:
	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_1_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_2_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_3_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_1_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_2_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_3_IsEditable;
};

/**
 * A struct for a property that has one double range component per DOF (Degree Of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangePropertyPerDof
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_1_IsEditable"))
	FAGX_RealInterval Translational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_2_IsEditable"))
	FAGX_RealInterval Translational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_3_IsEditable"))
	FAGX_RealInterval Translational_3;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_1_IsEditable"))
	FAGX_RealInterval Rotational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_2_IsEditable"))
	FAGX_RealInterval Rotational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_3_IsEditable"))
	FAGX_RealInterval Rotational_3;

	FAGX_ConstraintRangePropertyPerDof(
		double DefaultMinValue = 0.0, double DefaultMaxValue = 0.0,
		EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: Translational_1(DefaultMinValue, DefaultMaxValue)
		, Translational_2(DefaultMinValue, DefaultMaxValue)
		, Translational_3(DefaultMinValue, DefaultMaxValue)
		, Rotational_1(DefaultMinValue, DefaultMaxValue)
		, Rotational_2(DefaultMinValue, DefaultMaxValue)
		, Rotational_3(DefaultMinValue, DefaultMaxValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational3)
	{
	}

	FAGX_ConstraintRangePropertyPerDof(
		FAGX_RealInterval DefaultInterval, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: FAGX_ConstraintRangePropertyPerDof(DefaultInterval.Min, DefaultInterval.Max, EditableDofs)
	{
	}

	FAGX_RealInterval& operator[](int32 Index)
	{
		check(Index >= 0 && Index < 6);
		return (&Translational_1)[Index];
	}

	FAGX_RealInterval& operator[](EGenericDofIndex Index)
	{
		return operator[](static_cast<int32>(Index));
	};

	FAGX_RealInterval operator[](int32 Index) const
	{
		check(Index >= 0 && Index < 6);
		return (&Translational_1)[Index];
	}

	FAGX_RealInterval operator[](EGenericDofIndex Index) const
	{
		return operator[](static_cast<int32>(Index));
	};

private:
	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_1_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_2_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Translational_3_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_1_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_2_IsEditable;

	UPROPERTY(Transient, Category = "AGX Constraint Property Per Dof", VisibleDefaultsOnly)
	bool Rotational_3_IsEditable;
};
