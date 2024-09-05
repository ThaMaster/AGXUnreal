// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/LidarOutputBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LidarModel.h>
#include <agxSensor/LidarRayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceDistanceGaussianNoise.h>
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

namespace LidarBarrier_helpers
{
	class UnrealLidarModel : public agxSensor::LidarModel
	{
	public:
		UnrealLidarModel(agxSensor::LidarRayPatternGenerator* PatternGenerator)
			: LidarModel(PatternGenerator, new agxSensor::LidarRayRange())
		{
		}
	};

	class GenericHorizontalSweepModel : public agxSensor::HorizontalSweepLidarModel
	{
	public:
		GenericHorizontalSweepModel()
			: HorizontalSweepLidarModel(
				  {agx::degreesToRadians(360.0), agx::degreesToRadians(50.0)},
				  {agx::degreesToRadians(1.0), agx::degreesToRadians(1.0)}, 10.0)
		{
			getRayRange()->setRange({0.0, 100.0});
		}
	};

	agxSensor::LidarModelRef CreateTemporaryLidarModelFrom(EAGX_LidarModel InModel)
	{
		switch (InModel)
		{
			case EAGX_LidarModel::Custom:
				UE_LOG(
					LogAGX, Error,
					TEXT("Custom Lidar Model passed to "
						 "FLidarBarrier::CreateTemporaryLidarModelFrom. The Custom case should be "
						 "handled elsewhere, returning nullptr."));
				return nullptr;
				break;
			case EAGX_LidarModel::GenericHorizontalSweep:
				return new GenericHorizontalSweepModel();
				break;
		}

		UE_LOG(
			LogAGX, Error,
			TEXT("Unknown Lidar Model passed to "
				 "FLidarBarrier::CreateTemporaryLidarModelFrom. Returning nullptr."));
		return nullptr;
	}
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

	using namespace LidarBarrier_helpers;
	NativeRef->Native = new agxSensor::Lidar(
		nullptr, new UnrealLidarModel(new FCustomPatternGenerator(PatternFetcher)));
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
	size_t GenerateUniqueOutputId()
	{
		static size_t Id = 1;
		return Id++;
	}
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

void FLidarBarrier::EnableDistanceGaussianNoise(double Mean, double StdDev, double StdDevSlope)
{
	check(HasNative());
	if (DistanceNoiseNativeRef == nullptr)
	{
		DistanceNoiseNativeRef = std::make_unique<FDistanceGaussianNoiseRef>();
		NativeRef->Native->getOutputHandler()->add(DistanceNoiseNativeRef->Native);
	}

	const agx::Real MeanAGX = ConvertDistanceToAGX(Mean);
	const agx::Real StdDevAGX = ConvertDistanceToAGX(StdDev);
	const agx::Real StdDevSlopeAGX = StdDevSlope; // Unitless.

	DistanceNoiseNativeRef->Native->setMean(MeanAGX);
	DistanceNoiseNativeRef->Native->setStdDevBase(StdDevAGX);
	DistanceNoiseNativeRef->Native->setStdDevSlope(StdDevSlopeAGX);
	DistanceNoiseNativeRef->Native->setDirty(true);
}

void FLidarBarrier::DisableDistanceGaussianNoise()
{
	check(HasNative());
	if (DistanceNoiseNativeRef == nullptr)
		return;

	NativeRef->Native->getOutputHandler()->remove(DistanceNoiseNativeRef->Native);
	DistanceNoiseNativeRef = nullptr;
}

bool FLidarBarrier::IsDistanceGaussianNoiseEnabled() const
{
	check(HasNative());
	return DistanceNoiseNativeRef != nullptr;
}

void FLidarBarrier::AddOutput(FLidarOutputBarrier& Output)
{
	check(HasNative());
	check(Output.HasNative());

	NativeRef->Native->getOutputHandler()->add(
		LidarBarrier_helpers::GenerateUniqueOutputId(), Output.GetNative()->Native);
}

FAGX_RealInterval FLidarBarrier::GetRangeFrom(EAGX_LidarModel InModel)
{
	if (auto Model = LidarBarrier_helpers::CreateTemporaryLidarModelFrom(InModel))
	{
		return ConvertDistance(Model->getRayRange()->getRange());
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FLidarBarrier::GetRangeFrom failed since no agxSensor::LidarModel could be created "
			 "from the passed EAGX_LidarModel."));
	return FAGX_RealInterval();
}

FAGX_Real FLidarBarrier::GetBeamDivergenceFrom(EAGX_LidarModel InModel)
{
	if (auto Model = LidarBarrier_helpers::CreateTemporaryLidarModelFrom(InModel))
	{
		return ConvertAngleToUnreal<double>(Model->getProperties()->getBeamDivergence());
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FLidarBarrier::GetBeamDivergenceFrom failed since no agxSensor::LidarModel could be created "
			 "from the passed EAGX_LidarModel."));
	return 0.0;
}

FAGX_Real FLidarBarrier::GetBeamExitRadiusFrom(EAGX_LidarModel InModel)
{
	if (auto Model = LidarBarrier_helpers::CreateTemporaryLidarModelFrom(InModel))
	{
		return ConvertDistanceToUnreal<FAGX_Real>(Model->getProperties()->getBeamExitRadius());
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FLidarBarrier::GetBeamExitRadiusFrom failed since no agxSensor::LidarModel could be created "
			 "from the passed EAGX_LidarModel."));
	return 0.0;
}

bool FLidarBarrier::GetEnableDistanceGaussianNoiseFrom(EAGX_LidarModel InModel)
{
	if (auto Model = LidarBarrier_helpers::CreateTemporaryLidarModelFrom(InModel))
	{
		for (const auto& Noise : Model->getOutputNoises())
		{
			if (Noise->as<agxSensor::RtDistanceGaussianNoise>() != nullptr)
				return true;
		}

		return false;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FLidarBarrier::GetEnableDistanceGaussianNoiseFrom failed since no agxSensor::LidarModel could be created "
			 "from the passed EAGX_LidarModel."));
	return false;
}

TOptional<FAGX_DistanceGaussianNoiseSettings> FLidarBarrier::GetDistanceGaussianNoiseFrom(
	EAGX_LidarModel InModel)
{
	if (auto Model = LidarBarrier_helpers::CreateTemporaryLidarModelFrom(InModel))
	{
		for (const auto& Noise : Model->getOutputNoises())
		{
			if (auto* DistanceNoiseAGX = Noise->as<agxSensor::RtDistanceGaussianNoise>())
			{
				FAGX_DistanceGaussianNoiseSettings Dgn;
				Dgn.Mean = ConvertDistanceToUnreal<double>(DistanceNoiseAGX->getMean());
				Dgn.StandardDeviation =
					ConvertDistanceToUnreal<double>(DistanceNoiseAGX->getStdDevBase());
				Dgn.StandardDeviationSlope = DistanceNoiseAGX->getStdDevSlope(); // Unitless.
				return Dgn;
			}
		}

		return {};
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FLidarBarrier::GetRangeFrom failed since no agxSensor::LidarModel could be created "
			 "from the passed EAGX_LidarModel."));
	return {};
}
