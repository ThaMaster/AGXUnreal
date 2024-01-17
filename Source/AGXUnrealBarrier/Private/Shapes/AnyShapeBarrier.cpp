// Copyright 2024, Algoryx Simulation AB.

#include "Shapes/AnyShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"

FAnyShapeBarrier::FAnyShapeBarrier()
	: FShapeBarrier()
{
}

FAnyShapeBarrier::FAnyShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
}

FAnyShapeBarrier::FAnyShapeBarrier(FAnyShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FAnyShapeBarrier::~FAnyShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FAnyShapeRef.
}

void FAnyShapeBarrier::AllocateNativeShape()
{
	UE_LOG(
		LogAGX, Error,
		TEXT("AllocateNativeShape called on an FAnyShapeBarrier. This is not supported because "
			 "this type doesn't know what type of AGX Dynamics shape to allocate."));
}

void FAnyShapeBarrier::ReleaseNativeShape()
{
	// While an AnyShapeBarrier cannot allocate a shape it can be given one, so this release
	// function must be properly implemented.
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
