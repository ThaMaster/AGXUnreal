// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AMOR/AGX_WireMergeSplitThresholds.h"
#include "Wire/AGX_WireComponent.h"


void FAGX_WireMergeSplitProperties::OnBeginPlay(UAGX_WireComponent& Owner)
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
void FAGX_WireMergeSplitProperties::OnPostEditChangeProperty(UAGX_WireComponent& Owner)
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

void FAGX_WireMergeSplitProperties::CreateNative(UAGX_WireComponent& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());
	
	NativeBarrier.AllocateNative(*Owner.GetNative());
}

FAGX_WireMergeSplitProperties& FAGX_WireMergeSplitProperties::operator=(
	const FAGX_WireMergeSplitProperties& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

void FAGX_WireMergeSplitProperties::UpdateNativeProperties(UAGX_WireComponent& Owner)
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);

	UpdateNativeThresholds(Owner.GetWorld());
}

void FAGX_WireMergeSplitProperties::UpdateNativeThresholds(UWorld* PlayingWorld)
{
	AGX_CHECK(HasNative());
	if (Thresholds == nullptr)
	{
		NativeBarrier.SetWireMergeSplitThresholds(nullptr);
		return;
	}

	UAGX_WireMergeSplitThresholds* ThresholdsInstance =
			Thresholds->GetOrCreateInstance(PlayingWorld);
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

	FWireMergeSplitThresholdsBarrier* Barrier =
		ThresholdsInstance->GetOrCreateNative(PlayingWorld);
	AGX_CHECK(Barrier);

	NativeBarrier.SetWireMergeSplitThresholds(Barrier);
}
