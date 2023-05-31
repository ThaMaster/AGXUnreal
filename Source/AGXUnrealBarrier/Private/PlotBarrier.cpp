// Copyright 2023, Algoryx Simulation AB.

#include "PlotBarrier.h"

#include "AGXRefs.h"

FPlotBarrier::FPlotBarrier()
	: NativeRef {new FPlotRef}
{
}

FPlotBarrier::FPlotBarrier(std::unique_ptr<FPlotRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FPlotBarrier::FPlotBarrier(FPlotBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FPlotRef);
}

FPlotBarrier::~FPlotBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FPlotRef.
}

bool FPlotBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FPlotBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxPlot::System();
}

FPlotRef* FPlotBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FPlotRef* FPlotBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FPlotBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
