// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarResultBase.h"
#include "Sensors/AGX_LidarResultTypes.h"
#include "Sensors/LidarResultPositionIntensityBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarResultPositionIntensity.generated.h"

class FLidarResultBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarResultPositionIntensity : public FAGX_LidarResultBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarResultPositionIntensity() = default;

	void DebugDrawResult(
		UAGX_LidarSensorComponent* Lidar, float LifeTime = 0.12f, float Size = 6.f);

	virtual bool HasNative() const override;
	virtual FLidarResultBarrier* GetOrCreateNative() override;
	virtual const FLidarResultBarrier* GetNative() const override;

	void GetResult(TArray<FAGX_LidarResultPositionIntensityData>& OutResult);

	// We must provide operator = because the Unreal framework will attempt to invoke it.
	FAGX_LidarResultPositionIntensity& operator=(const FAGX_LidarResultPositionIntensity& Other);
	bool operator==(const FAGX_LidarResultPositionIntensity& Other) const;

private:
	FLidarResultPositionIntensityBarrier NativeBarrier;
	TArray<FAGX_LidarResultPositionIntensityData> Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarResultPositionIntensity in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarResultPositionIntensity_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AssociateWith(
		UPARAM(ref) FAGX_LidarResultPositionIntensity& Result, UAGX_LidarSensorComponent* Lidar)
	{
		Result.AssociateWith(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetResult(
		UPARAM(ref) FAGX_LidarResultPositionIntensity& Result,
		TArray<FAGX_LidarResultPositionIntensityData>& OutResult)
	{
		Result.GetResult(OutResult);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void DebugDrawResult(
		UPARAM(ref) FAGX_LidarResultPositionIntensity& Result, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float Size = 6.f)
	{
		Result.DebugDrawResult(Lidar, LifeTime, Size);
	}
};
