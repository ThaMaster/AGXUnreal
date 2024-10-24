// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BrickAsset.generated.h"


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_BrickAsset : public UObject
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditAnywhere, Category = "AGX Brick")
	int Dummy;
};

