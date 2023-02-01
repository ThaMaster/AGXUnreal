// Copyright 2023, Algoryx Simulation AB.

#include "Shapes/EmptyShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

FEmptyShapeBarrier::FEmptyShapeBarrier()
	: FShapeBarrier()
{
}

FEmptyShapeBarrier::FEmptyShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape == nullptr);
}

FEmptyShapeBarrier::FEmptyShapeBarrier(FEmptyShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FEmptyShapeBarrier::~FEmptyShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FEmptyShapeRef.
}

bool FEmptyShapeBarrier::HasNative() const
{
	return NativeRef->NativeGeometry != nullptr;
}

void FEmptyShapeBarrier::AllocateNativeShape()
{
}

void FEmptyShapeBarrier::ReleaseNativeShape()
{
}
