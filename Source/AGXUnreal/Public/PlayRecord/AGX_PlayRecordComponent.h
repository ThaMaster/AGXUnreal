// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"


#include "AGX_PlayRecordComponent.generated.h"

class UAGX_PlayRecord;

/**
 * Todo: add description
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlayRecordComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlayRecordComponent();

	UPROPERTY(EditAnywhere, Category = "AGX Play Record")
	UAGX_PlayRecord* PlayRecord = nullptr;
};
