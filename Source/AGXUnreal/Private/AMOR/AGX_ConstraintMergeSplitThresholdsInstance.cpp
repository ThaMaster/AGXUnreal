// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsInstance.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Simulation.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholdsAsset.h"

UAGX_ConstraintMergeSplitThresholdsBase*
UAGX_ConstraintMergeSplitThresholdsInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FConstraintMergeSplitThresholdsBarrier());

	NativeBarrier->AllocateNative();
	AGX_CHECK(HasNative());

	UpdateNativeProperties();
}

FConstraintMergeSplitThresholdsBarrier*
UAGX_ConstraintMergeSplitThresholdsInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}

	return NativeBarrier.Get();
}

bool UAGX_ConstraintMergeSplitThresholdsInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

UAGX_ConstraintMergeSplitThresholdsInstance*
UAGX_ConstraintMergeSplitThresholdsInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholdsAsset& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	AGX_CHECK(Outer);

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ConstraintMergeSplitThresholdsInstance>(
		Outer, UAGX_ConstraintMergeSplitThresholdsInstance::StaticClass(), *InstanceName,
		RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::CopyProperties(
	UAGX_ConstraintMergeSplitThresholdsAsset& Source)
{
	// Todo: implement.
}

void UAGX_ConstraintMergeSplitThresholdsInstance::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	// TODO: implement.
}

void UAGX_ConstraintMergeSplitThresholdsInstance::SetMaxDesiredForceRangeDiff(
	FAGX_Real InMaxDesiredForceRangeDiff)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxDesiredForceRangeDiff(MaxDesiredForceRangeDiff);
	}

	MaxDesiredForceRangeDiff = InMaxDesiredForceRangeDiff;
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsInstance::GetMaxDesiredForceRangeDiff() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxDesiredForceRangeDiff();
	}

	return MaxDesiredForceRangeDiff;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::SetMaxDesiredLockAngleDiff(
	FAGX_Real InMaxDesiredLockAngleDiff)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxDesiredLockAngleDiff(MaxDesiredLockAngleDiff);
	}

	MaxDesiredLockAngleDiff = InMaxDesiredLockAngleDiff;
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsInstance::GetMaxDesiredLockAngleDiff() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxDesiredLockAngleDiff();
	}

	return MaxDesiredLockAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::SetMaxDesiredRangeAngleDiff(
	FAGX_Real InMaxDesiredRangeAngleDiff)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxDesiredRangeAngleDiff(MaxDesiredRangeAngleDiff);
	}

	MaxDesiredRangeAngleDiff = InMaxDesiredRangeAngleDiff;
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsInstance::GetMaxDesiredRangeAngleDiff() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxDesiredRangeAngleDiff();
	}

	return MaxDesiredRangeAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::SetMaxDesiredSpeedDiff(
	FAGX_Real InMaxDesiredSpeedDiff)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxDesiredSpeedDiff(MaxDesiredSpeedDiff);
	}

	MaxDesiredSpeedDiff = InMaxDesiredSpeedDiff;
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsInstance::GetMaxDesiredSpeedDiff() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxDesiredSpeedDiff();
	}

	return MaxDesiredSpeedDiff;
}

void UAGX_ConstraintMergeSplitThresholdsInstance::SetMaxRelativeSpeed(
	FAGX_Real InMaxRelativeSpeed)
{
	if (HasNative())
	{
		NativeBarrier->SetMaxRelativeSpeed(MaxRelativeSpeed);
	}

	MaxRelativeSpeed = InMaxRelativeSpeed;
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsInstance::GetMaxRelativeSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier->GetMaxRelativeSpeed();
	}

	return MaxRelativeSpeed;
}