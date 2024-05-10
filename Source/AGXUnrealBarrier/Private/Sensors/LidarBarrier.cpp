// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/LidarOutputBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LidarRayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceOutput.h>
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

void FLidarBarrier::AllocateNativeLidarRayPatternHorizontalSweep(
	const FVector2D& FOV, const FVector2D& Resolution, double Frequency)
{
	check(!HasNative());

	const agx::Vec2 FovAGX {ConvertAngleToAGX(FOV.X), ConvertAngleToAGX(FOV.Y)};
	const agx::Vec2 ResolutionAGX {
		ConvertAngleToAGX(Resolution.X), ConvertAngleToAGX(Resolution.Y)};

	NativeRef->Native = new agxSensor::Lidar(
		nullptr, new agxSensor::HorizontalSweepLidarModel(FovAGX, ResolutionAGX, Frequency));
}

void FLidarBarrier::AllocateNativeRayPatternCustom(FCustomPatternFetcherBase* PatternFetcher)
{
	check(!HasNative());

	// TODO!!
	// NativeRef->Native = new agxSensor::Lidar(nullptr, new
	// FCustomPatternGenerator(PatternFetcher));
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
	NativeRef->Native->getModel()->getRayRange()->setRange(
		{static_cast<float>(ConvertDistanceToAGX(Range.Min)),
		 static_cast<float>(ConvertDistanceToAGX(Range.Max))});
}

FAGX_RealInterval FLidarBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal32 RangeAGX = NativeRef->Native->getModel()->getRayRange()->getRange();
	return FAGX_RealInterval(
		ConvertDistanceToUnreal<double>(RangeAGX.lower()),
		ConvertDistanceToUnreal<double>(RangeAGX.upper()));
}

void FLidarBarrier::SetBeamDivergence(double BeamDivergence)
{
	check(HasNative());
	const agx::Real DivergenceAGX = ConvertAngleToAGX(BeamDivergence);
	NativeRef->Native->getModel()->getProperties()->setBeamDivergence(DivergenceAGX);
}

double FLidarBarrier::GetBeamDivergence() const
{
	check(HasNative());
	const agx::Real DivergenceAGX =
		NativeRef->Native->getModel()->getProperties()->getBeamDivergence();
	return ConvertAngleToUnreal<double>(DivergenceAGX);
}

void FLidarBarrier::SetBeamExitRadius(double BeamExitRadius)
{
	check(HasNative());
	const agx::Real ExitRadiusAGX = ConvertDistanceToAGX(BeamExitRadius);
	NativeRef->Native->getModel()->getProperties()->setBeamExitRadius(ExitRadiusAGX);
}

double FLidarBarrier::GetBeamExitRadius() const
{
	check(HasNative());
	const agx::Real ExitRadiusAGX =
		NativeRef->Native->getModel()->getProperties()->getBeamExitRadius();
	return ConvertDistanceToUnreal<double>(ExitRadiusAGX);
}

namespace LidarBarrier_helpers
{
	size_t GenerateUniqueResultId()
	{
		static size_t Id = 1;
		return Id++;
	}
}

bool FLidarBarrier::EnableDistanceGaussianNoise(double Mean, double StdDev, double StdDevSlope)
{
	// check(HasNative());
	// const agx::Real MeanAGX = ConvertDistanceToAGX(Mean);
	// const agx::Real StdDevAGX = ConvertDistanceToAGX(StdDev);
	// const agx::Real StdDevSlopeAGX = StdDevSlope; // Unitless.
	// return NativeRef->Native->enableDistanceGaussianNoise(
	//	MeanAGX, StdDevAGX,
	//	StdDevSlopeAGX);
	return false; // TODO
}

bool FLidarBarrier::DisableDistanceGaussianNoise()
{
	/*check(HasNative());
	return NativeRef->Native->disableDistanceGaussianNoise();*/
	return false; // TODO
}

bool FLidarBarrier::IsDistanceGaussianNoiseEnabled() const
{
	/*check(HasNative());
	return NativeRef->Native->getEnableDistanceGaussianNoise();*/
	return false; // TODO
}

void FLidarBarrier::SetEnableRemoveRayMisses(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getOutputHandler()->setEnableRemoveRayMisses(bEnable);
}

bool FLidarBarrier::GetEnableRemoveRayMisses() const
{
	check(HasNative());
	return NativeRef->Native->getOutputHandler()->getEnableRemoveRayMisses();
}

void FLidarBarrier::AddResult(FLidarOutputBarrier& Result)
{
	check(HasNative());
	check(Result.HasNative());

	NativeRef->Native->getOutputHandler()->add(
		LidarBarrier_helpers::GenerateUniqueResultId(), Result.GetNative()->Native);
}
