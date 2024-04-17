// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarResultBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"


FLidarResultBarrier::FLidarResultBarrier()
	: NativeRef {new FLidarResultRef}
{
}

FLidarResultBarrier::FLidarResultBarrier(std::unique_ptr<FLidarResultRef> Native)
	: NativeRef(std::move(Native))
{
}

FLidarResultBarrier::FLidarResultBarrier(FLidarResultBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FLidarResultRef);
}

FLidarResultBarrier::~FLidarResultBarrier()
{
}

bool FLidarResultBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FLidarResultRef* FLidarResultBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FLidarResultRef* FLidarResultBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FLidarResultBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}
