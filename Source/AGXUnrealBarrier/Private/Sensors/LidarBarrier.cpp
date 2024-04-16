// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include <agxSensor/RayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceResult.h>

namespace FLidarBarrier_helpers
{
	agxSensor::RayPatternGeneratorRef CreatePatternGenerator(
		EAGX_LidarRayPattern Pattern, FCustomPatternFetcherBase* PatternFetcher)
	{
		if (Pattern == EAGX_LidarRayPattern::HorizontalSweep)
		{
			// Todo: make these configurable in the Lidar Sensor.
			const agx::Vec2 fov {agx::degreesToRadians(360.0), agx::degreesToRadians(50.0)};
			const agx::Vec2 resolution {agx::degreesToRadians(1.0)};
			const agx::Real frequency = 2.0;
			return new agxSensor::RayPatternHorizontalSweep(fov, resolution, frequency);
		}

		if (Pattern == EAGX_LidarRayPattern::Custom)
		{
			return new FCustomPatternGenerator(PatternFetcher);
		}

		UE_LOG(
			LogAGX, Error,
			TEXT("Unknown Lidar Scan Pattern given to "
				 "FLidarBarrier_helpers::CreatePatternGenerator()."));
		return nullptr;
	}
}

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

void FLidarBarrier::AllocateNative(
	EAGX_LidarRayPattern Pattern, FCustomPatternFetcherBase* PatternFetcher)
{
	using namespace FLidarBarrier_helpers;
	check(!HasNative());

	NativeRef->Native =
		new agxSensor::Lidar(nullptr, CreatePatternGenerator(Pattern, PatternFetcher));

	// Todo: make this configurable in the Lidar sensor!
	NativeRef->Native->getResultHandler()
		->add<agx::Vec4f, agxSensor::RtResult::XYZ_VEC3_F32, agxSensor::RtResult::INTENSITY_F32>();
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

void FLidarBarrier::SetRange(FAGX_RealInterval Range)
{
	check(HasNative());
	NativeRef->Native->getRayRangeNode()->setRange(
		{static_cast<float>(ConvertDistanceToAGX(Range.Min)),
		 static_cast<float>(ConvertDistanceToAGX(Range.Max))});
}

FAGX_RealInterval FLidarBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal32 RangeAGX = NativeRef->Native->getRayRangeNode()->getRange();
	return FAGX_RealInterval(
		ConvertDistanceToUnreal<double>(RangeAGX.lower()),
		ConvertDistanceToUnreal<double>(RangeAGX.upper()));
}

void FLidarBarrier::SetBeamDivergence(double BeamDivergence)
{
	check(HasNative());
	agxSensor::LidarSettings Settings = NativeRef->Native->getLidarSettings();
	Settings.beamDivergence = ConvertAngleToAGX(BeamDivergence);
	NativeRef->Native->setLidarSettings(Settings);
}

double FLidarBarrier::GetBeamDivergence() const
{
	check(HasNative());
	const agxSensor::LidarSettings& Settings = NativeRef->Native->getLidarSettings();
	return ConvertAngleToUnreal<double>(Settings.beamDivergence);
}

void FLidarBarrier::SetBeamExitDiameter(double BeamExitDiameter)
{
	check(HasNative());
	agxSensor::LidarSettings Settings = NativeRef->Native->getLidarSettings();
	Settings.beamExitDiameter = ConvertDistanceToAGX(BeamExitDiameter);
	NativeRef->Native->setLidarSettings(Settings);
}

double FLidarBarrier::GetBeamExitDiameter() const
{
	check(HasNative());
	const agxSensor::LidarSettings& Settings = NativeRef->Native->getLidarSettings();
	return ConvertDistanceToUnreal<double>(Settings.beamExitDiameter);
}

#include "DrawDebugHelpers.h"

void FLidarBarrier::GetResultTest(UWorld* World, const FTransform& Transform)
{
	const auto dataView = NativeRef->Native->getResultHandler()->view<agx::Vec4f>();

	if (World != nullptr)
	{
		for (const agx::Vec4f& ResultAGX : dataView)
		{
			const FVector PointLocal =
				ConvertDisplacement(agx::Vec3(ResultAGX.x(), ResultAGX.y(), ResultAGX.z()));
			const FVector Point = Transform.TransformPositionNoScale(PointLocal);
			if (Point.GetMax() > 10000)
				continue;
			DrawDebugPoint(World, Point, 6.f, FColor::Red, false, 0.12f);
		}
	}
}
