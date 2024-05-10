// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/LidarOutputPositionBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarOutputPosition.generated.h"

class FLidarOutputBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarOutputPosition : public FAGX_LidarOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarOutputPosition() = default;

	void DebugDrawResult(
		UAGX_LidarSensorComponent* Lidar, float LifeTime = 0.12f, float Size = 6.f,
		FColor Color = FColor::Red);

	virtual bool HasNative() const override;
	virtual FLidarOutputBarrier* GetOrCreateNative() override;
	virtual const FLidarOutputBarrier* GetNative() const override;

	void GetResult(TArray<FAGX_LidarOutputPositionData>& OutResult);

	FAGX_LidarOutputPosition& operator=(const FAGX_LidarOutputPosition& Other);
	bool operator==(const FAGX_LidarOutputPosition& Other) const;

private:
	FLidarOutputPositionBarrier NativeBarrier;
	TArray<FAGX_LidarOutputPositionData> Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarOutputPosition in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarOutputPosition_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AddTo(
		UPARAM(ref) FAGX_LidarOutputPosition& Result, UAGX_LidarSensorComponent* Lidar)
	{
		Result.AddTo(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetResult(
		UPARAM(ref) FAGX_LidarOutputPosition& Result,
		TArray<FAGX_LidarOutputPositionData>& OutResult)
	{
		Result.GetResult(OutResult);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void DebugDrawResult(
		UPARAM(ref) FAGX_LidarOutputPosition& Result, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float Size = 6.f, FLinearColor Color = FLinearColor::Red)
	{
		Result.DebugDrawResult(Lidar, LifeTime, Size, Color.ToFColor(false));
	}
};
