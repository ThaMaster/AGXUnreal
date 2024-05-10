// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/LidarOutputPositionIntensityBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarOutputPositionIntensity.generated.h"

class FLidarOutputBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarOutputPositionIntensity : public FAGX_LidarOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarOutputPositionIntensity() = default;

	void DebugDrawResult(
		UAGX_LidarSensorComponent* Lidar, float LifeTime = 0.12f, float Size = 6.f);

	virtual bool HasNative() const override;
	virtual FLidarOutputBarrier* GetOrCreateNative() override;
	virtual const FLidarOutputBarrier* GetNative() const override;

	void GetResult(TArray<FAGX_LidarOutputPositionIntensityData>& OutResult);

	FAGX_LidarOutputPositionIntensity& operator=(const FAGX_LidarOutputPositionIntensity& Other);
	bool operator==(const FAGX_LidarOutputPositionIntensity& Other) const;

private:
	FLidarOutputPositionIntensityBarrier NativeBarrier;
	TArray<FAGX_LidarOutputPositionIntensityData> Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarOutputPositionIntensity in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarOutputPositionIntensity_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AddTo(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Result, UAGX_LidarSensorComponent* Lidar)
	{
		Result.AddTo(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetResult(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Result,
		TArray<FAGX_LidarOutputPositionIntensityData>& OutResult)
	{
		Result.GetResult(OutResult);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void DebugDrawResult(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Result, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float Size = 6.f)
	{
		Result.DebugDrawResult(Lidar, LifeTime, Size);
	}
};
