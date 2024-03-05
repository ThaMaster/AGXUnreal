// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include <agxSensor/RayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceResult.h>

FLidarBarrier::FLidarBarrier()
	: NativeRef {new FLidarRef}
{
}

FLidarBarrier::FLidarBarrier(std::unique_ptr<FLidarRef> Native)
	: NativeRef(std::move(Native))
{
}

FLidarBarrier::FLidarBarrier(FLidarBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FLidarRef);
}

FLidarBarrier::~FLidarBarrier()
{
	ReleaseNative();
}

bool FLidarBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FLidarBarrier::AllocateNative()
{
	check(!HasNative());
	const agx::Vec2 fov {agx::degreesToRadians(360.0), agx::degreesToRadians(50.0)};
	const agx::Vec2 resolution {agx::degreesToRadians(1.0)};
	const agx::Real frequency = 2.0;

	NativeRef->Native = new agxSensor::Lidar(
		nullptr, new agxSensor::RayPatternHorizontalSweep(fov, resolution, frequency));

	NativeRef->Native->getResultHandler()
		->add<agx::Vec4f, agxSensor::RtResult::XYZ_VEC3_F32, agxSensor::RtResult::DISTANCE_F32>();
}

FLidarRef* FLidarBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FLidarRef* FLidarBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FLidarBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}

void FLidarBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	*NativeRef->Native->getFrame() =
		*ConvertFrame(Transform.GetLocation(), Transform.GetRotation());
}

#include "DrawDebugHelpers.h"

void FLidarBarrier::GetResultTest(UWorld* World)
{
	const auto dataView = NativeRef->Native->getResultHandler()->view<agx::Vec4f>();
	
	if (World != nullptr)
	{
		for (const agx::Vec4f& ResultAGX : dataView)
		{
			agx::Vec3f PAGX(ResultAGX.x(), ResultAGX.y(), ResultAGX.z());
			DrawDebugPoint(World, ConvertDisplacement(PAGX), 6.f, FColor::Red, false, 0.12f);
		}
	}
}
