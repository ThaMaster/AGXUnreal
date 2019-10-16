#pragma once

#include "CoreMinimal.h"


#define DEFAULT_SECONDARY_COMPLIANCE 1.0e-10
#define DEFAULT_SECONDARY_ELASTICITY (1.0 / DEFAULT_SECONDARY_COMPLIANCE)
#define DEFAULT_SECONDARY_DAMPING (2.0 / 60.0)
#define RANGE_LOWEST_FLOAT TNumericLimits<float>::Lowest()
#define RANGE_HIGHEST_FLOAT TNumericLimits<float>::Max()

