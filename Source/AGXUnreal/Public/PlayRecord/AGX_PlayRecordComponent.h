// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_PlayRecordComponent.generated.h"

class UAGX_ConstraintComponent;
class UAGX_PlayRecord;

/**
 * EXPERIMENTAL
 * 
 * This Component enables simple Constraint position recording and playback functionality.
 * It does not guarantee equivalent constraint forces/torques during playback.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlayRecordComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlayRecordComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	UPROPERTY(EditAnywhere, Category = "AGX Play Record")
	UAGX_PlayRecord* PlayRecord = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Play Record Advanced")
	int32 InitialStatesAllocationSize {3600};

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics Play Record")
	void RecordConstraintPositions(const TArray<UAGX_ConstraintComponent*>& Constraints);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics Play Record")
	void PlayBackConstraintPositions(const TArray<UAGX_ConstraintComponent*>& Constraints);

private:
	int32 CurrentIndex {0};
};
