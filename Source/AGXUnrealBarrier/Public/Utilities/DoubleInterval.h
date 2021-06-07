#pragma once

// Unreal Engine includes.
#include "Math/Interval.h"

#include "DoubleInterval.generated.h"

#if 0

/// @todo Replace this with the Unreal Engine double interval once they provide one.
/// This type is mostly useless since it can't be used as a UPROPERTY since it's not a USTRUCT.
/// They must be doing something custom with FFloatInterval.
/// We can perhaps do a double interval from scratch, with the USTRUCT decorator. Unsure how much
/// GUI-work that would entail.
DEFINE_INTERVAL_WRAPPER_STRUCT(FAGX_DoubleInterval, double)

#else

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_DoubleInterval
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Interval")
	double Min;

	UPROPERTY(EditAnywhere, Category = "Interval")
	double Max;
};


#endif
