#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

#include "RenderMaterial.generated.h"

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_RenderMaterial
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
	FVector Ambient;

	UPROPERTY()
	FVector Diffuse;

	UPROPERTY()
	FVector Emissive;

	UPROPERTY()
	float Shininess;
};
