// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXSignalHandler.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"

#include "PLX_SignalHandlerComponent.generated.h"

/**
 * Todo: Add description.
 */
UCLASS(
	ClassGroup = "OpenPLX", Category = "OpenPLX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UPLX_SignalHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPLX_SignalHandlerComponent();

	UPROPERTY(EditAnywhere, Category = "OpenPLX")
	TMap<FString, FPLX_Input> Inputs;

	UPROPERTY(EditAnywhere, Category = "OpenPLX")
	TMap<FString, FPLX_Output> Outputs;

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool SendScalar(const FPLX_Input& Input, double Value);

	UFUNCTION(BlueprintCallable, Category = "OpenPLX")
	bool ReceiveScalar(const FPLX_Output& Output, double& OutValue);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

private:
	FPLXSignalHandler SignalHandler;
};
