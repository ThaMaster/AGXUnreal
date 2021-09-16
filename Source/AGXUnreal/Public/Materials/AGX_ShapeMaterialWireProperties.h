#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialWireProperties.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialWireProperties
{
	GENERATED_BODY()

public:
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulusStretch;

	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double DampingStretch;

	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulusBend;

	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double DampingBend;

public:
	FAGX_ShapeMaterialWireProperties();
};
