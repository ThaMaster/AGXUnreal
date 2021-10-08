#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialWireProperties.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialWireProperties
{
	GENERATED_BODY()

public:

	/**
	 * Young's modulus when stretching the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulusStretch;

	/**
	 * Damping (spook) when stretching the wire [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double SpookDampingStretch;

	/**
	 * Young's modulus when bending the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulusBend;


	/**
	 * Damping (spook) when bending the wire [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double SpookDampingBend;

public:
	FAGX_ShapeMaterialWireProperties();
};
