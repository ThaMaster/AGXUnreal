#pragma once

#include "CoreMinimal.h"
#include "AGX_CollisionGroupPair.generated.h"

USTRUCT()
struct FAGX_CollisionGroupPair
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group1;

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group2;
};
