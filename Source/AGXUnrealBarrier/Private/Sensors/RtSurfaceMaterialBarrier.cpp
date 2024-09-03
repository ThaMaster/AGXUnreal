// Copyright 2024, Algoryx Simulation AB.


#include "Sensors/RtSurfaceMaterialBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceSurfaceMaterial.h>
#include "EndAGXIncludes.h"

FRtSurfaceMaterialBarrier::FRtSurfaceMaterialBarrier()
	: Native(new FRtSurfaceMaterial())
{
}

FRtSurfaceMaterialBarrier::FRtSurfaceMaterialBarrier(std::unique_ptr<FRtSurfaceMaterial> InNative)
	: Native(std::move(InNative))
{
}

FRtSurfaceMaterialBarrier::FRtSurfaceMaterialBarrier(FRtSurfaceMaterialBarrier&& Other)
	: Native {std::move(Other.Native)}
{
	Other.Native.reset(new FRtSurfaceMaterial());
}

FRtSurfaceMaterialBarrier::~FRtSurfaceMaterialBarrier()
{
}

void FRtSurfaceMaterialBarrier::SetReflectivity(float Reflectivity)
{
	check(HasNative());
	Native->Native.setReflectivity(Reflectivity);
}

float FRtSurfaceMaterialBarrier::GetReflectivity() const
{
	check(HasNative());
	return Native->Native.getReflectivity();
}

void FRtSurfaceMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	Native->Native = agxSensor::RtSurfaceMaterial::create();
}

bool FRtSurfaceMaterialBarrier::HasNative() const
{
	return Native->Native.isValid();
}

FRtSurfaceMaterial* FRtSurfaceMaterialBarrier::GetNative()
{
	check(HasNative());
	return Native.get();
}

const FRtSurfaceMaterial* FRtSurfaceMaterialBarrier::GetNative() const
{
	check(HasNative());
	return Native.get();
}

void FRtSurfaceMaterialBarrier::ReleaseNative()
{
	Native = nullptr;
}
