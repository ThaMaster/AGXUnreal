// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_GeometryContactMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Wire/AGX_WireComponent.h"


template <typename T>
void FAGX_GeometryContactMergeSplitProperties::OnBeginPlay(T& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());

	// Only allocate native if either EnableMerge or EnableSplit is true.
	// Not having a native is a perfectly valid and regular thing for this class.
	if (bEnableMerge || bEnableSplit)
	{
		NativeBarrier.AllocateNative(*Owner.GetNative());
		UpdateNativeProperties();
	}
}

#if WITH_EDITOR
template <typename T>
void FAGX_GeometryContactMergeSplitProperties::OnPostEditChangeProperty(T& Owner)
{
	// If we have not yet allocated a native, and we are in Play, and EnableMerge or EnableSplit
	// is true, then we should now allocate a Native.
	if (Owner.HasNative() && !HasNative() && (bEnableMerge || bEnableSplit))
	{
		NativeBarrier.AllocateNative(*Owner.GetNative());
	}

	if (HasNative())
	{
		UpdateNativeProperties();
	}
}
#endif

template <typename T>
void FAGX_GeometryContactMergeSplitProperties::CreateNative(T& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());
	
	NativeBarrier.AllocateNative(*Owner.GetNative());
	UpdateNativeProperties();
}

FAGX_GeometryContactMergeSplitProperties& FAGX_GeometryContactMergeSplitProperties::operator=(
	const FAGX_GeometryContactMergeSplitProperties& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

void FAGX_GeometryContactMergeSplitProperties::SetEnableMerge(bool bEnable)
{
	bEnableMerge = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableMerge(bEnable);
	}
}

bool FAGX_GeometryContactMergeSplitProperties::GetEnableMerge() const
{
	return bEnableMerge;
}

void FAGX_GeometryContactMergeSplitProperties::SetEnableSplit(bool bEnable)
{
	bEnableSplit = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableSplit(bEnable);
	}
}

bool FAGX_GeometryContactMergeSplitProperties::GetEnableSplit() const
{
	return bEnableSplit;
}

bool FAGX_GeometryContactMergeSplitProperties::HasNative() const
{
	return NativeBarrier.HasNative();
	return true;
}

const FMergeSplitPropertiesBarrier& FAGX_GeometryContactMergeSplitProperties::GetNative() const
{
	return NativeBarrier;
}

FMergeSplitPropertiesBarrier& FAGX_GeometryContactMergeSplitProperties::GetNative()
{
	return NativeBarrier;
}

void FAGX_GeometryContactMergeSplitProperties::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);
}

// Explicit template instantiations.
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnBeginPlay<UAGX_RigidBodyComponent>(
	UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnBeginPlay<UAGX_ConstraintComponent>(
	UAGX_ConstraintComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnBeginPlay<UAGX_ShapeComponent>(
	UAGX_ShapeComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnBeginPlay<UAGX_WireComponent>(
	UAGX_WireComponent&);

#if WITH_EDITOR
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnPostEditChangeProperty<
	UAGX_RigidBodyComponent>(UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnPostEditChangeProperty<
	UAGX_ConstraintComponent>(UAGX_ConstraintComponent&);
template AGXUNREAL_API void
FAGX_GeometryContactMergeSplitProperties::OnPostEditChangeProperty<UAGX_ShapeComponent>(UAGX_ShapeComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::OnPostEditChangeProperty<UAGX_WireComponent>(
	UAGX_WireComponent&);
#endif

template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::CreateNative<UAGX_RigidBodyComponent>(
	UAGX_RigidBodyComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::CreateNative<UAGX_ConstraintComponent>(
	UAGX_ConstraintComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::CreateNative<UAGX_ShapeComponent>(
	UAGX_ShapeComponent&);
template AGXUNREAL_API void FAGX_GeometryContactMergeSplitProperties::CreateNative<UAGX_WireComponent>(
	UAGX_WireComponent&);
