// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarResultBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"


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

bool FLidarResultBarrier::EnableDistanceGaussianNoise(
	double Mean, double StdDev, double StdDevSlope)
{
	check(HasNative());
	const agx::Real MeanAGX = ConvertDistanceToAGX(Mean);
	const agx::Real StdDevAGX = ConvertDistanceToAGX(StdDev);
	const agx::Real StdDevSlopeAGX = StdDevSlope; // Unitless.
	return NativeRef->Native->enableDistanceGaussianNoise(MeanAGX, StdDevAGX, StdDevSlopeAGX);
}

bool FLidarResultBarrier::DisableDistanceGaussianNoise()
{
	check(HasNative());
	return NativeRef->Native->disableDistanceGaussianNoise();
}

bool FLidarResultBarrier::IsDistanceGaussianNoiseEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableDistanceGaussianNoise();
}

bool FLidarResultBarrier::EnableRemovePointsMisses(bool bEnable)
{
	check(HasNative());
	return NativeRef->Native->enableRemovePointMisses();
}

bool FLidarResultBarrier::IsRemovePointsMissesEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableRemovePointMisses();
}
