// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "UObject/Class.h"

namespace AGX_ConstraintMergeSplitProperties_helpers
{
	void CheckAmorEnabled()
	{
		const UAGX_Simulation* Simulation = GetDefault<UAGX_Simulation>();
		if (Simulation == nullptr)
		{
			return;
		}

		if (!Simulation->bEnableAMOR)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AMOR enabled on a Constraint, but disabled globally. Enable AMOR in Project "
					 "Settings > Plugins > AGX Dynamics for this change to have an effect."));
		}
	}
}

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
	if (bEnableMerge || bEnableSplit)
	{
		AGX_ConstraintMergeSplitProperties_helpers::CheckAmorEnabled();
		if (Owner.HasNative() && !HasNative())
		{
			// If we have not yet allocated a native, and we are in Play, and EnableMerge or
			// EnableSplit is true, then we should now allocate a Native.
			CreateNative(Owner);
		}
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

void FAGX_ConstraintMergeSplitProperties::BindBarrierToOwner(FConstraintBarrier& NewOwner)
{
	if (NewOwner.HasNative())
	{
		NativeBarrier.BindToNewOwner(NewOwner);
	}
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

UAGX_MergeSplitThresholdsBase* FAGX_ConstraintMergeSplitProperties::GetThresholds()
{
	return Thresholds;
}