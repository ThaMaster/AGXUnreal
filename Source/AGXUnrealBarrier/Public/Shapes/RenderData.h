#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

#include "RenderData.generated.h"

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_RenderData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector DiffuseColor;

	/// @todo Add the rest of the render data properties.

	/// @todo Properties are optional, so some form of Has.+ must be added.
};
