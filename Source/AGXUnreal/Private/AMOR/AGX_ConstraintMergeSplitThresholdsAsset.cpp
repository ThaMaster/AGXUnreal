// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsAsset.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholdsInstance.h"

UAGX_ConstraintMergeSplitThresholdsBase*
UAGX_ConstraintMergeSplitThresholdsAsset::GetOrCreateInstance(
	UWorld* PlayingWorld, bool bIsRotational)
{
	UAGX_ConstraintMergeSplitThresholdsInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_ConstraintMergeSplitThresholdsInstance::CreateFromAsset(PlayingWorld, *this, bIsRotational);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

#if WITH_EDITOR
void UAGX_ConstraintMergeSplitThresholdsAsset::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ConstraintMergeSplitThresholdsAsset::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ConstraintMergeSplitThresholdsAsset::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholdsBase, MaxDesiredForceRangeDiff),
		[](ThisClass* This) { This->SetMaxDesiredForceRangeDiff(This->MaxDesiredForceRangeDiff); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholdsBase, MaxDesiredLockAngleDiff),
		[](ThisClass* This) { This->SetMaxDesiredLockAngleDiff(This->MaxDesiredLockAngleDiff); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholdsBase, MaxDesiredRangeAngleDiff),
		[](ThisClass* This) { This->SetMaxDesiredRangeAngleDiff(This->MaxDesiredRangeAngleDiff); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholdsBase, MaxDesiredSpeedDiff),
		[](ThisClass* This) { This->SetMaxDesiredSpeedDiff(This->MaxDesiredSpeedDiff); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholdsBase, MaxRelativeSpeed),
		[](ThisClass* This) { This->SetMaxRelativeSpeed(This->MaxRelativeSpeed); });
}
#endif

void UAGX_ConstraintMergeSplitThresholdsAsset::SetMaxDesiredForceRangeDiff(
	FAGX_Real InMaxDesiredForceRangeDiff)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxDesiredForceRangeDiff(MaxDesiredForceRangeDiff);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxDesiredForceRangeDiff = InMaxDesiredForceRangeDiff;
	}
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsAsset::GetMaxDesiredForceRangeDiff() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxDesiredForceRangeDiff();
	}

	return MaxDesiredForceRangeDiff;
}

void UAGX_ConstraintMergeSplitThresholdsAsset::SetMaxDesiredLockAngleDiff(
	FAGX_Real InMaxDesiredLockAngleDiff)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxDesiredLockAngleDiff(MaxDesiredLockAngleDiff);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxDesiredLockAngleDiff = InMaxDesiredLockAngleDiff;
	}
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsAsset::GetMaxDesiredLockAngleDiff() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxDesiredLockAngleDiff();
	}

	return MaxDesiredLockAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsAsset::SetMaxDesiredRangeAngleDiff(
	FAGX_Real InMaxDesiredRangeAngleDiff)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxDesiredRangeAngleDiff(MaxDesiredRangeAngleDiff);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxDesiredRangeAngleDiff = InMaxDesiredRangeAngleDiff;
	}
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsAsset::GetMaxDesiredRangeAngleDiff() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxDesiredRangeAngleDiff();
	}

	return MaxDesiredRangeAngleDiff;
}

void UAGX_ConstraintMergeSplitThresholdsAsset::SetMaxDesiredSpeedDiff(
	FAGX_Real InMaxDesiredSpeedDiff)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxDesiredSpeedDiff(MaxDesiredSpeedDiff);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxDesiredSpeedDiff = InMaxDesiredSpeedDiff;
	}
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsAsset::GetMaxDesiredSpeedDiff() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxDesiredSpeedDiff();
	}

	return MaxDesiredSpeedDiff;
}

void UAGX_ConstraintMergeSplitThresholdsAsset::SetMaxRelativeSpeed(
	FAGX_Real InMaxRelativeSpeed)
{
	if (Instance != nullptr)
	{
		Instance->SetMaxRelativeSpeed(MaxRelativeSpeed);
	}
	else
	{
		// If no instance exist (we are not in Play), we allow writing permanently to this asset.
		MaxRelativeSpeed = InMaxRelativeSpeed;
	}
}

FAGX_Real UAGX_ConstraintMergeSplitThresholdsAsset::GetMaxRelativeSpeed() const
{
	if (Instance != nullptr)
	{
		return Instance->GetMaxRelativeSpeed();
	}

	return MaxRelativeSpeed;
}