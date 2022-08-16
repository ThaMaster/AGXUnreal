// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsInstance.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Wire/AGX_WireComponent.h"


template <typename T>
void FAGX_ShapeContactMergeSplitProperties::OnBeginPlay(T& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());

	// Only allocate native if either EnableMerge or EnableSplit is true.
	// Not having a native is a perfectly valid and regular thing for this class.
	if (bEnableMerge || bEnableSplit)
	{
		CreateNative(Owner);
	}

	if (Thresholds != nullptr)
	{
		SwapThresholdsAssetToInstance(Owner.GetWorld());
	}
}

#if WITH_EDITOR
template <typename T>
void FAGX_ShapeContactMergeSplitProperties::OnPostEditChangeProperty(T& Owner)
{
	// If we have not yet allocated a native, and we are in Play, and EnableMerge or EnableSplit
	// is true, then we should now allocate a Native.
	if (Owner.HasNative() && !HasNative() && (bEnableMerge || bEnableSplit))
	{
		CreateNative(Owner);
	}
}
#endif

template <typename T>
void FAGX_ShapeContactMergeSplitProperties::CreateNative(T& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());
	
	NativeBarrier.AllocateNative(*Owner.GetNative());
	UpdateNativeProperties();
}

FAGX_ShapeContactMergeSplitProperties& FAGX_ShapeContactMergeSplitProperties::operator=(
	const FAGX_ShapeContactMergeSplitProperties& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

void FAGX_ShapeContactMergeSplitProperties::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);
}

void FAGX_ShapeContactMergeSplitProperties::SwapThresholdsAssetToInstance(UWorld* PlayingWorld)
{
	if (Thresholds == nullptr)
	{
		return;
	}

	UAGX_ShapeContactMergeSplitThresholdsInstance* ThresholdsInstance =
		static_cast<UAGX_ShapeContactMergeSplitThresholdsInstance*>(
			Thresholds->GetOrCreateInstance(PlayingWorld));
	if (!ThresholdsInstance)
	{
		UE_LOG(LogAGX, Warning, TEXT("Unable to create a Merge Split Thresholds instance from the "
		"given asset '%s'."), *Thresholds->GetName());
		return;
	}

	if (Thresholds != ThresholdsInstance && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		// Perform the Asset to Instance swap.
		Thresholds = ThresholdsInstance;
	}
	
	FShapeContactMergeSplitThresholdsBarrier* Barrier =
		ThresholdsInstance->GetOrCreateNative(PlayingWorld);
	AGX_CHECK(Barrier);

	NativeBarrier.SetConstraintMergeSplitThresholds(Barrier);
}

// Explicit template instantiations.
template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::OnBeginPlay<UAGX_RigidBodyComponent>(
	UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::OnBeginPlay<UAGX_ShapeComponent>(
	UAGX_ShapeComponent&);

#if WITH_EDITOR
template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::OnPostEditChangeProperty<
	UAGX_RigidBodyComponent>(UAGX_RigidBodyComponent&);
template AGXUNREAL_API void
FAGX_ShapeContactMergeSplitProperties::OnPostEditChangeProperty<UAGX_ShapeComponent>(UAGX_ShapeComponent&);
#endif

template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::CreateNative<UAGX_RigidBodyComponent>(
	UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::CreateNative<UAGX_ShapeComponent>(
	UAGX_ShapeComponent&);
