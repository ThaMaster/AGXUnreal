#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Misc/Guid.h"

#include "RenderMaterial.generated.h"

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_RenderMaterial
{
	GENERATED_BODY()

	FAGX_RenderMaterial()
		: bHasAmbient(0)
		, bHasDiffuse(0)
		, bHasEmissive(0)
		, bHasShininess(0)
	{
	}

	// Bit-fields cannot have in-class/inline initializer so set in constructor instead.

	UPROPERTY()
	uint8 bHasAmbient : 1;

	UPROPERTY()
	uint8 bHasDiffuse : 1;

	UPROPERTY()
	uint8 bHasEmissive : 1;

	UPROPERTY()
	uint8 bHasShininess : 1;

	// Initializing a FVector4 from FVector::ZeroVector gives us black with full opacity.

	UPROPERTY()
	FVector4 Ambient = FVector::ZeroVector;

	UPROPERTY()
	FVector4 Diffuse = FVector::ZeroVector;

	UPROPERTY()
	FVector4 Emissive = FVector::ZeroVector;

	UPROPERTY()
	float Shininess = 0.0f;

	UPROPERTY()
	FName Name;

	FGuid Guid;

	/**
	 * Convert an sRGB 0..1 color, for example those stored in FAGX_RenderMaterial, to FLinearColor
	 * for use in Unreal Engine Render Material parameters.
	 *
	 * @return The given color in the linear color space.
	 */
	static FLinearColor ConvertToLinear(const FVector4& SRGBColor);
};
