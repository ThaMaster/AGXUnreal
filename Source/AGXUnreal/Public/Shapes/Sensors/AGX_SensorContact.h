#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_SensorContact.generated.h"

/**
 * TODO: add description
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_SensorContact
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Sensor Contacts")
	float Dummy;

	FAGX_SensorContact() = default;
};
