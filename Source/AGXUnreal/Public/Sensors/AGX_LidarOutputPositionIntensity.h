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

	void DebugDrawData(
		const TArray<FAGX_LidarOutputPositionIntensityData>& Data, UAGX_LidarSensorComponent* Lidar,
		float LifeTime, float BaseSize);

	virtual bool HasNative() const override;
	virtual FLidarOutputBarrier* GetOrCreateNative() override;
	virtual const FLidarOutputBarrier* GetNative() const override;

	void GetData(TArray<FAGX_LidarOutputPositionIntensityData>& OutData);

	FAGX_LidarOutputPositionIntensity& operator=(const FAGX_LidarOutputPositionIntensity& Other);
	bool operator==(const FAGX_LidarOutputPositionIntensity& Other) const;

private:
	FLidarOutputPositionIntensityBarrier NativeBarrier;
	TArray<FAGX_LidarOutputPositionIntensityData> Data;
	TArray<FVector> RenderPositions;
	TArray<FLinearColor> RenderColors;
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
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output, UAGX_LidarSensorComponent* Lidar)
	{
		Output.AddTo(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetData(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output,
		TArray<FAGX_LidarOutputPositionIntensityData>& OutData)
	{
		Output.GetData(OutData);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void DebugDrawData(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output,
		const TArray<FAGX_LidarOutputPositionIntensityData>& Data, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float BaseSize = 5.f)
	{
		Output.DebugDrawData(Data, Lidar, LifeTime, BaseSize);
	}
};
