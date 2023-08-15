// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PlayRecord.generated.h"


UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_PlayRecord : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics Play Record")
	int32 Dummy;
};
