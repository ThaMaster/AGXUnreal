// Copyright 2022, Algoryx Simulation AB.

#include "MergeSplitPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Wire/WireBarrier.h"
#include "Wire/WireRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/MergeSplitHandler.h>
#include "EndAGXIncludes.h"


FMergeSplitPropertiesBarrier::FMergeSplitPropertiesBarrier()
	: NativePtr {new FMergeSplitPropertiesPtr}
{
}

FMergeSplitPropertiesBarrier::FMergeSplitPropertiesBarrier(
	std::unique_ptr<FMergeSplitPropertiesPtr> Native)
	: NativePtr {std::move(Native)}
{
}

FMergeSplitPropertiesBarrier::FMergeSplitPropertiesBarrier(
	FMergeSplitPropertiesBarrier&& Other) noexcept
	: NativePtr {std::move(Other.NativePtr)}
{
	Other.NativePtr = std::make_unique<FMergeSplitPropertiesPtr>();
}

FMergeSplitPropertiesBarrier::~FMergeSplitPropertiesBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the std::unique_ptr
	// NativePtr's destructor must be able to see the definition, not just the declaration, of
	// FMergeSplitPropertiesBarrier.
}

bool FMergeSplitPropertiesBarrier::HasNative() const
{
	return NativePtr->Native != nullptr;
}

template <typename T>
void FMergeSplitPropertiesBarrier::AllocateNative(T& Owner)
{
	check(!HasNative());
	check(Owner.HasNative());
	NativePtr->Native = agxSDK::MergeSplitHandler::getOrCreateProperties(Owner.GetNative()->Native);
}

template<>
void FMergeSplitPropertiesBarrier::AllocateNative<FShapeBarrier>(FShapeBarrier& Owner)
{
	check(!HasNative());
	check(Owner.HasNative());
	NativePtr->Native =
		agxSDK::MergeSplitHandler::getOrCreateProperties(Owner.GetNative()->NativeGeometry);
}

FMergeSplitPropertiesPtr* FMergeSplitPropertiesBarrier::GetNative()
{
	check(HasNative());
	return NativePtr.get();
}

const FMergeSplitPropertiesPtr* FMergeSplitPropertiesBarrier::GetNative() const
{
	check(HasNative());
	return NativePtr.get();
}

void FMergeSplitPropertiesBarrier::SetEnableMerge(bool bEnable)
{
	check(HasNative());
	NativePtr->Native->setEnableMerge(bEnable);
}

bool FMergeSplitPropertiesBarrier::GetEnableMerge() const
{
	check(HasNative());
	return NativePtr->Native->getEnableMerge();
}

void FMergeSplitPropertiesBarrier::SetEnableSplit(bool bEnable)
{
	check(HasNative());
	NativePtr->Native->setEnableSplit(bEnable);
}

bool FMergeSplitPropertiesBarrier::GetEnableSplit() const
{
	check(HasNative());
	return NativePtr->Native->getEnableSplit();
}

// Explicit template instantiations.
template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FRigidBodyBarrier>(
	FRigidBodyBarrier&);

template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FConstraintBarrier>(
	FConstraintBarrier&);

template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FShapeBarrier>(
	FShapeBarrier&);

template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FWireBarrier>(
	FWireBarrier&);
