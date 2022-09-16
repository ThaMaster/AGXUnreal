// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"


void FAGX_ConstraintMergeSplitProperties::OnBeginPlay(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());

	// Only allocate native if either EnableMerge or EnableSplit is true.
	// Not having a native is a perfectly valid and regular thing for this class.
	if (bEnableMerge || bEnableSplit)
	{
		CreateNative(Owner);
		UpdateNativeProperties(Owner);
	}
}

#if WITH_EDITOR
void FAGX_ConstraintMergeSplitProperties::OnPostEditChangeProperty(UAGX_ConstraintComponent& Owner)
{
	// If we have not yet allocated a native, and we are in Play, and EnableMerge or EnableSplit
	// is true, then we should now allocate a Native.
	if (Owner.HasNative() && !HasNative() && (bEnableMerge || bEnableSplit))
	{
		CreateNative(Owner);
	}

	if (HasNative())
	{
		UpdateNativeProperties(Owner);
	}
}
#endif

void FAGX_ConstraintMergeSplitProperties::CreateNative(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());
	
	NativeBarrier.AllocateNative(*Owner.GetNative());
}

void FAGX_ConstraintMergeSplitProperties::CopyFrom(
	const FMergeSplitPropertiesBarrier& Barrier, UAGX_MergeSplitThresholdsBase* InThresholds)
{
	bEnableMerge = Barrier.GetEnableMerge();
	bEnableSplit = Barrier.GetEnableSplit();

	Thresholds = Cast<UAGX_ConstraintMergeSplitThresholds>(InThresholds);
}

FAGX_ConstraintMergeSplitProperties& FAGX_ConstraintMergeSplitProperties::operator=(
	const FAGX_ConstraintMergeSplitProperties& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

void FAGX_ConstraintMergeSplitProperties::UpdateNativeProperties(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);

	UpdateNativeThresholds(Owner);
}

void FAGX_ConstraintMergeSplitProperties::UpdateNativeThresholds(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(HasNative());
	if (Thresholds == nullptr)
	{
		NativeBarrier.SetConstraintMergeSplitThresholds(nullptr);
		return;
	}

	UWorld* PlayingWorld = Owner.GetWorld();
	UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =		
			Thresholds->GetOrCreateInstance(PlayingWorld, Owner.IsRotational());
	if (!ThresholdsInstance)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create a Merge Split Thresholds instance from the "
				 "given asset '%s'."),
			*Thresholds->GetName());
		return;
	}

	if (Thresholds != ThresholdsInstance)
	{
		Thresholds = ThresholdsInstance;
	}

	FConstraintMergeSplitThresholdsBarrier* Barrier =
		ThresholdsInstance->GetOrCreateNative(PlayingWorld, Owner.IsRotational());
	AGX_CHECK(Barrier);

	NativeBarrier.SetConstraintMergeSplitThresholds(Barrier);
}