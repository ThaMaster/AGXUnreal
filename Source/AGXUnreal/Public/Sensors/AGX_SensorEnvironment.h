// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/RtEntityBarrier.h"
#include "Sensors/RtMeshBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/AGX_LidarSensorReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_SensorEnvironment.generated.h"

class UStaticMeshComponent;

UCLASS(ClassGroup = "AGX", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_SensorEnvironment : public AActor
{
	GENERATED_BODY()

public:
	AAGX_SensorEnvironment();

	UPROPERTY(EditAnywhere, Category = "AGX Sensor Environment")
	TArray<FAGX_LidarSensorReference> Lidars;

	/*
	 * Add a Static Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool Add(UStaticMeshComponent* Mesh);

	bool Add(
		UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	/*
	 * Remove a Static Mesh Component from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool Remove(UStaticMeshComponent* Mesh);

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
	struct FMeshEntityBarrierData
	{
		FRtMeshBarrier Mesh;
		FRtEntityBarrier Entity;
		FTransform Transform;
	};

	TMap<UStaticMeshComponent*, FMeshEntityBarrierData> StaticMeshes;
	FSensorEnvironmentBarrier NativeBarrier;
};
