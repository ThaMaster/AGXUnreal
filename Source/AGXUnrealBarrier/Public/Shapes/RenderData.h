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
	uint8 bHasAmbient : 1;

	UPROPERTY()
	uint8 bHasDiffuse : 1;

	UPROPERTY()
	uint8 bHasEmissive : 1;

	UPROPERTY()
	uint8 bHasShininess : 1;

	UPROPERTY()
	FVector AmbientColor;

	UPROPERTY()
	FVector DiffuseColor;

	UPROPERTY()
	FVector EmissiveColor;

	UPROPERTY()
	float Shininess;


	/// @todo Add the rest of the render data properties.

	/// @todo Properties are optional, so some form of Has.+ must be added.
};
