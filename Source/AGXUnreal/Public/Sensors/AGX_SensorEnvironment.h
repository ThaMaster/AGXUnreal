// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/AGX_LidarSensorReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_SensorEnvironment.generated.h"


UCLASS(ClassGroup = "AGX", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_SensorEnvironment : public AActor
{
	GENERATED_BODY()

public:
	AAGX_SensorEnvironment();

	UPROPERTY(EditAnywhere, Category = "AGX Sensor Environment")
	TArray<FAGX_LidarSensorReference> Lidars;

	bool HasNative() const;

	FSensorEnvironmentBarrier* GetNative();
	const FSensorEnvironmentBarrier* GetNative() const;

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// ~End UObject interface.

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FSensorEnvironmentBarrier NativeBarrier;
};
