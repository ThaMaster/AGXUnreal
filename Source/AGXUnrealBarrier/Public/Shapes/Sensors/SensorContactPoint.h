#pragma once

// Unreal Engine includes.
#include "Containers/Array.h"

struct AGXUNREALBARRIER_API FSensorContactPoint
{
	FVector Position;
	FVector Force;
	FVector NomalForce;
	FVector Normal;
	float Depth;
	float Area;
};
