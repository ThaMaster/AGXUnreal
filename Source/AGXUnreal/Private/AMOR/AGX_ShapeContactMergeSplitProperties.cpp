// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "Shapes/AGX_ShapeComponent.h"


namespace AGX_ShapeContactMergeSplitProperties_helpers
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
				TEXT("AMOR enabled on a Body or Shape, but disabled globally. Enable "
					 "AMOR in Project Settings > Plugins > AGX Dynamics for this change "
					 "to have "
					 "an effect."));
		}
	}
}

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
		UpdateNativeProperties(Owner);
	}
}

#if WITH_EDITOR
template <typename T>
void FAGX_ShapeContactMergeSplitProperties::OnPostEditChangeProperty(T& Owner)
{
	if (bEnableMerge || bEnableSplit)
	{
		AGX_ShapeContactMergeSplitProperties_helpers::CheckAmorEnabled();

		// If we have not yet allocated a native, and we are in Play, and EnableMerge or EnableSplit
		// is true, then we should now allocate a Native.
		if (Owner.HasNative() && !HasNative())
		{
			CreateNative(Owner);
		}
	}	

	if (HasNative())
	{
		UpdateNativeProperties(Owner);
	}
}
#endif

template <typename T>
void FAGX_ShapeContactMergeSplitProperties::CreateNative(T& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());
	
	NativeBarrier.AllocateNative(*Owner.GetNative());
}

template <typename T>
void FAGX_ShapeContactMergeSplitProperties::UpdateNativeProperties(T& Owner)
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);

	UpdateNativeThresholds(Owner.GetWorld());
}

void FAGX_ShapeContactMergeSplitProperties::UpdateNativeThresholds(UWorld* PlayingWorld)
{
	AGX_CHECK(HasNative());
	if (Thresholds == nullptr)
	{
		NativeBarrier.SetShapeContactMergeSplitThresholds(nullptr);
		return;
	}

	UAGX_ShapeContactMergeSplitThresholds* ThresholdsInstance =		
			Thresholds->GetOrCreateInstance(PlayingWorld);
	if (!ThresholdsInstance)
	{
		UE_LOG(LogAGX, Warning, TEXT("Unable to create a Merge Split Thresholds instance from the "
		"given asset '%s'."), *Thresholds->GetName());
		return;
	}

	if (Thresholds != ThresholdsInstance)
	{
		Thresholds = ThresholdsInstance;
	}
	
	FShapeContactMergeSplitThresholdsBarrier* Barrier =
		ThresholdsInstance->GetOrCreateNative(PlayingWorld);
	AGX_CHECK(Barrier);

	NativeBarrier.SetShapeContactMergeSplitThresholds(Barrier);
}

void FAGX_ShapeContactMergeSplitProperties::BindBarrierToOwner(FRigidBodyBarrier& NewOwner)
{
	if (NewOwner.HasNative())
	{
		NativeBarrier.BindToNewOwner(NewOwner);
	}
}

void FAGX_ShapeContactMergeSplitProperties::BindBarrierToOwner(FShapeBarrier& NewOwner)
{
	if (NewOwner.HasNative())
	{
		NativeBarrier.BindToNewOwner(NewOwner);
	}
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

template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::UpdateNativeProperties<
	UAGX_RigidBodyComponent>(UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_ShapeContactMergeSplitProperties::UpdateNativeProperties<
	UAGX_ShapeComponent>(UAGX_ShapeComponent&);
