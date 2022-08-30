// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsBase.h"

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredForceRangeDiff_AsFloat(
	float InMaxDesiredForceRangeDiff)
{
	SetMaxDesiredForceRangeDiff(FAGX_Real(InMaxDesiredForceRangeDiff));
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredForceRangeDiff(
	FAGX_Real InMaxDesiredForceRangeDiff)
{
	MaxDesiredForceRangeDiff = InMaxDesiredForceRangeDiff;
}

float UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredForceRangeDiff_AsFloat() const
{
	return static_cast<float>(MaxDesiredForceRangeDiff);
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredForceRangeDiff() const
{
	return MaxDesiredForceRangeDiff;
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredLockAngleDiff_AsFloat(
	float InMaxDesiredLockAngleDiff)
{
	SetMaxDesiredLockAngleDiff(FAGX_Real(InMaxDesiredLockAngleDiff));
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredLockAngleDiff(
	FAGX_Real InMaxDesiredLockAngleDiff)
{
	MaxDesiredLockAngleDiff = InMaxDesiredLockAngleDiff;
}

float UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredLockAngleDiff_AsFloat() const
{
	return static_cast<float>(MaxDesiredLockAngleDiff);
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredLockAngleDiff() const
{
	return MaxDesiredLockAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredRangeAngleDiff_AsFloat(
	float InMaxDesiredRangeAngleDiff)
{
	SetMaxDesiredRangeAngleDiff(FAGX_Real(InMaxDesiredRangeAngleDiff));
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredRangeAngleDiff(
	FAGX_Real InMaxDesiredRangeAngleDiff)
{
	MaxDesiredRangeAngleDiff = InMaxDesiredRangeAngleDiff;
}

float UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredRangeAngleDiff_AsFloat() const
{
	return static_cast<float>(MaxDesiredRangeAngleDiff);
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredRangeAngleDiff() const
{
	return MaxDesiredRangeAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredSpeedDiff_AsFloat(
	float InMaxDesiredSpeedDiff)
{
	SetMaxDesiredSpeedDiff(FAGX_Real(InMaxDesiredSpeedDiff));
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxDesiredSpeedDiff(
	FAGX_Real InMaxDesiredSpeedDiff)
{
	MaxDesiredSpeedDiff = InMaxDesiredSpeedDiff;
}

float UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredSpeedDiff_AsFloat() const
{
	return static_cast<float>(MaxDesiredSpeedDiff);
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsBase::GetMaxDesiredSpeedDiff() const
{
	return MaxDesiredSpeedDiff;
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxRelativeSpeed_AsFloat(float InMaxRelativeSpeed)
{
	SetMaxRelativeSpeed(FAGX_Real(InMaxRelativeSpeed));
}

void UAGX_ConstraintMergeSplitThresholdsBase::SetMaxRelativeSpeed(FAGX_Real InMaxRelativeSpeed)
{
	MaxRelativeSpeed = InMaxRelativeSpeed;
}

float UAGX_ConstraintMergeSplitThresholdsBase::GetMaxRelativeSpeed_AsFloat() const
{
	return static_cast<float>(MaxRelativeSpeed);
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsBase::GetMaxRelativeSpeed() const
{
	return MaxRelativeSpeed;
}