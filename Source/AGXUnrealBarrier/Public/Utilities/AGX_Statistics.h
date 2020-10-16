#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_Statistics.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAGX_Statistics {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics")
	float StepForwardTime = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics")
	int32 NumBodies = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics")
	int32 NumConstraints = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Statistics")
	int32 NumParticles = -1;
};
