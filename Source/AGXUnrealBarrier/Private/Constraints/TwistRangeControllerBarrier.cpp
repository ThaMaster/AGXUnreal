// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/TwistRangeControllerBarrier.h"

#if 0

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"
#include "AGXRefs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

//
// Twist Range Contorller starts here.
//

// Special member functions.

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier()
	: NativeRef(new FTwistRangeControllerRef())
{
}

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier(
	const FTwistRangeControllerBarrier& Other)
	: NativeRef(new FTwistRangeControllerRef(Other.NativeRef->Native))
{
}

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier(
	std::unique_ptr<FTwistRangeControllerRef> Native)
	: NativeRef(std::move(Native))
{
}

FTwistRangeControllerBarrier::~FTwistRangeControllerBarrier()
{
	// Must have a non-inlined destructor because the NativeRef destructor must be able to see the
	// full definition of the pointed-to type, and we are not allowed to include F*Ref / F*Ptr
	// types in the header file.
}

FTwistRangeControllerBarrier& FTwistRangeControllerBarrier::operator=(
	const FTwistRangeControllerBarrier& Other)
{
	NativeRef->Native = Other.NativeRef->Native;
	return *this;
}

// AGX Dynamics accessors.

void FTwistRangeControllerBarrier::SetEnabled(bool bInEnabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(bInEnabled);
}

bool FTwistRangeControllerBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FTwistRangeControllerBarrier::SetRange(FDoubleInterval InRange)
{
	check(HasNative());
	const agx::RangeReal RangeAGX = ConvertAngle(InRange);
	NativeRef->Native->setRange(RangeAGX);
}

FDoubleInterval FTwistRangeControllerBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal RangeAGX = NativeRef->Native->getRange();
	const FDoubleInterval Range = ConvertAngle(RangeAGX);
	return Range;
}

// Native management.

bool FTwistRangeControllerBarrier::HasNative() const
{
	check(NativeRef != nullptr);
	return NativeRef->Native != nullptr;
}

FTwistRangeControllerRef* FTwistRangeControllerBarrier::GetNative()
{
	return NativeRef.get();
}

const FTwistRangeControllerRef* FTwistRangeControllerBarrier::GetNative() const
{
	return NativeRef.get();
}

#endif
