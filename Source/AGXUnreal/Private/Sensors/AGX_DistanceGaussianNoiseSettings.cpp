// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"

bool FAGX_DistanceGaussianNoiseSettings::operator==(
	const FAGX_DistanceGaussianNoiseSettings& Other) const
{
	return Mean == Other.Mean &&
		StandardDeviation == Other.StandardDeviation &&
		StandardDeviationSlope == Other.StandardDeviationSlope;
}
