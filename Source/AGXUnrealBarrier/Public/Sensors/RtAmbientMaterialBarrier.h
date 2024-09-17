// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtAmbientMaterial;

class AGXUNREALBARRIER_API FRtAmbientMaterialBarrier
{
public:
	FRtAmbientMaterialBarrier();
	FRtAmbientMaterialBarrier(std::unique_ptr<FRtAmbientMaterial> InNative);
	FRtAmbientMaterialBarrier(FRtAmbientMaterialBarrier&& Other);
	virtual ~FRtAmbientMaterialBarrier();

	virtual void AllocateNative();

	bool HasNative() const;
	FRtAmbientMaterial* GetNative();
	const FRtAmbientMaterial* GetNative() const;
	void ReleaseNative();

	void SetRefractiveIndex(float InRefractiveIndex);
	float GetRefractiveIndex() const;

	void SetAttenuationCoefficient(float InAttenuationCoefficient);
	float GetAttenuationCoefficient() const;

	void SetReturnGammaDistributionProbabilityScaling(float InScalingParameter);
	float GetReturnGammaDistributionProbabilityScaling() const;

	void SetReturnGammaDistributionShapeParameter(float InShapeParameter);
	float GetReturnGammaDistributionShapeParameter() const;

	void SetReturnGammaDistributionScaleParameter(float InScaleParameter);
	float GetReturnGammaDistributionScaleParameter() const;

private:
	FRtAmbientMaterialBarrier(const FRtAmbientMaterialBarrier&) = delete;
	void operator=(const FRtAmbientMaterialBarrier&) = delete;

protected:
	std::unique_ptr<FRtAmbientMaterial> Native;
};
