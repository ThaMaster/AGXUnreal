// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"

/**
 * Constants that reflect the initial values used in the native constraint implementations.
 * Therefore, do not change these!
 */
struct ConstraintConstants
{
public:
	static constexpr double DefaultCompliance()
	{
		return 1.0e-8;
	}
	static constexpr double DefaultElasticity()
	{
		return 1.0 / DefaultCompliance();
	};

	static constexpr double StrongCompliance()
	{
		return 1.0e-10;
	}
	static constexpr double StrongElasticity()
	{
		return 1.0 / StrongCompliance();
	};

	static constexpr double DefaultSpookDamping()
	{
		return 2.0 / 60.0;
	}

	static constexpr float FloatRangeMax()
	{
		return TNumericLimits<float>::Max();
	}
	static constexpr float FloatRangeMin()
	{
		return TNumericLimits<float>::Lowest();
	}
};
