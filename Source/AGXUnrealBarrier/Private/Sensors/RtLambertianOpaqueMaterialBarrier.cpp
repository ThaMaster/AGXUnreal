// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

FRtLambertianOpaqueMaterialBarrier::FRtLambertianOpaqueMaterialBarrier()
	: Native(new FRtLambertianOpaqueMaterial())
{
}

FRtLambertianOpaqueMaterialBarrier::FRtLambertianOpaqueMaterialBarrier(
	std::unique_ptr<FRtLambertianOpaqueMaterial> InNative)
	: Native(std::move(InNative))
{
}

FRtLambertianOpaqueMaterialBarrier::FRtLambertianOpaqueMaterialBarrier(
	FRtLambertianOpaqueMaterialBarrier&& Other)
	: Native {std::move(Other.Native)}
{
	Other.Native.reset(new FRtLambertianOpaqueMaterial());
}

FRtLambertianOpaqueMaterialBarrier::~FRtLambertianOpaqueMaterialBarrier()
{
}

void FRtLambertianOpaqueMaterialBarrier::SetReflectivity(float Reflectivity)
{
	check(HasNative());
	Native->Native.setReflectivity(Reflectivity);
}

float FRtLambertianOpaqueMaterialBarrier::GetReflectivity() const
{
	check(HasNative());
	return Native->Native.getReflectivity();
}

void FRtLambertianOpaqueMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	Native->Native = agxSensor::RtLambertianOpaqueMaterial::create();
}

bool FRtLambertianOpaqueMaterialBarrier::HasNative() const
{
	return Native->Native.isValid();
}

FRtLambertianOpaqueMaterial* FRtLambertianOpaqueMaterialBarrier::GetNative()
{
	check(HasNative());
	return Native.get();
}

const FRtLambertianOpaqueMaterial* FRtLambertianOpaqueMaterialBarrier::GetNative() const
{
	check(HasNative());
	return Native.get();
}

void FRtLambertianOpaqueMaterialBarrier::ReleaseNative()
{
	Native = nullptr;
}
