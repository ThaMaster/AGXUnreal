// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/LidarResultBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceResult.h>
#include "EndAGXIncludes.h"


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

void FLidarBarrier::AllocateNativeRayPatternHorizontalSweep(
	const FVector2D& FOV, const FVector2D& Resolution, double Frequency)
{
	check(!HasNative());

	const agx::Vec2 FovAGX {ConvertAngleToAGX(FOV.X), ConvertAngleToAGX(FOV.Y)};
	const agx::Vec2 ResolutionAGX {ConvertAngleToAGX(Resolution.X), ConvertAngleToAGX(Resolution.Y)};

	NativeRef->Native = new agxSensor::Lidar(
		nullptr, new agxSensor::RayPatternHorizontalSweep(FovAGX, ResolutionAGX, Frequency));

	// Todo: make this configurable in the Lidar sensor!
	NativeRef->Native->getResultHandler()
		->add<agx::Vec4f, agxSensor::RtResult::XYZ_VEC3_F32, agxSensor::RtResult::INTENSITY_F32>();
}

void FLidarBarrier::AllocateNativeRayPatternCustom(FCustomPatternFetcherBase* PatternFetcher)
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::Lidar(nullptr, new FCustomPatternGenerator(PatternFetcher));

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

namespace LidarBarrier_helpers
{
	size_t GenerateUniqueResultId()
	{
		static size_t Id = 1;
		return Id++;
	}
}

void FLidarBarrier::AddResult(FLidarResultBarrier& Result)
{
	check(HasNative());
	check(Result.HasNative());

	NativeRef->Native->getResultHandler()->add(
		LidarBarrier_helpers::GenerateUniqueResultId(), Result.GetNative()->Native);
}
