// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtAmbientMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

FRtAmbientMaterialBarrier::FRtAmbientMaterialBarrier()
	: Native(new FRtAmbientMaterial())
{
}

FRtAmbientMaterialBarrier::FRtAmbientMaterialBarrier(std::unique_ptr<FRtAmbientMaterial> InNative)
	: Native(std::move(InNative))
{
}

FRtAmbientMaterialBarrier::FRtAmbientMaterialBarrier(FRtAmbientMaterialBarrier&& Other)
	: Native {std::move(Other.Native)}
{
	Other.Native.reset(new FRtAmbientMaterial());
}

FRtAmbientMaterialBarrier::~FRtAmbientMaterialBarrier()
{
}

void FRtAmbientMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	Native->Native = agxSensor::RtAmbientMaterial::create();
}

bool FRtAmbientMaterialBarrier::HasNative() const
{
	return Native->Native.isValid();
}

FRtAmbientMaterial* FRtAmbientMaterialBarrier::GetNative()
{
	check(HasNative());
	return Native.get();
}

const FRtAmbientMaterial* FRtAmbientMaterialBarrier::GetNative() const
{
	check(HasNative());
	return Native.get();
}

void FRtAmbientMaterialBarrier::ReleaseNative()
{
	Native = nullptr;
}

void FRtAmbientMaterialBarrier::SetRefractiveIndex(float InRefractiveIndex)
{
	check(HasNative());
	Native->Native.setRefractiveIndex(InRefractiveIndex);
}

float FRtAmbientMaterialBarrier::GetRefractiveIndex() const
{
	check(HasNative());
	return Native->Native.getRefractiveIndex();
}

void FRtAmbientMaterialBarrier::SetAttenuationCoefficient(float InAttenuationCoefficient)
{
	check(HasNative());
	Native->Native.setAttenuationCoefficient(InAttenuationCoefficient);
}

float FRtAmbientMaterialBarrier::GetAttenuationCoefficient() const
{
	check(HasNative());
	return Native->Native.getAttenuationCoefficient();
}

void FRtAmbientMaterialBarrier::SetReturnGammaDistributionProbabilityScaling(
	float InScalingParameter)
{
	check(HasNative());
	Native->Native.setReturnGammaDistributionProbabilityScaling(InScalingParameter);
}

float FRtAmbientMaterialBarrier::GetReturnGammaDistributionProbabilityScaling() const
{
	check(HasNative());
	return Native->Native.getReturnGammaDistributionProbabilityScaling();
}

void FRtAmbientMaterialBarrier::SetReturnGammaDistributionShapeParameter(float InShapeParameter)
{
	check(HasNative());
	Native->Native.setReturnGammaDistributionShapeParameter(InShapeParameter);
}

float FRtAmbientMaterialBarrier::GetReturnGammaDistributionShapeParameter() const
{
	check(HasNative());
	return Native->Native.getReturnGammaDistributionShapeParameter();
}

void FRtAmbientMaterialBarrier::SetReturnGammaDistributionScaleParameter(float InScaleParameter)
{
	check(HasNative());
	Native->Native.setReturnGammaDistributionScaleParameter(InScaleParameter);
}

float FRtAmbientMaterialBarrier::GetReturnGammaDistributionScaleParameter() const
{
	check(HasNative());
	return Native->Native.getReturnGammaDistributionScaleParameter();
}
