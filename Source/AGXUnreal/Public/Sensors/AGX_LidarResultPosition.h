// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarResultBase.h"
#include "Sensors/AGX_LidarResultTypes.h"
#include "Sensors/LidarResultPositionBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarResultPosition.generated.h"

class FLidarResultBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarResultPosition : public FAGX_LidarResultBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarResultPosition() = default;

	// Making UAGX_LidarSensorComponent::AddResult Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	virtual bool AssociateWith(UAGX_LidarSensorComponent* Lidar) override;

	void DebugDrawResult(UAGX_LidarSensorComponent* Lidar);

	virtual bool HasNative() const override;
	virtual FLidarResultBarrier* GetOrCreateNative() override;
	virtual const FLidarResultBarrier* GetNative() const override;

	void GetResult(TArray<FAGX_LidarResultPositionData>& OutResult);

	// We must provide operator = because the Unreal framework will attempt to invoke it.
	FAGX_LidarResultPosition& operator=(const FAGX_LidarResultPosition& Other);
	bool operator==(const FAGX_LidarResultPosition& Other) const;

private:
	FLidarResultPositionBarrier NativeBarrier;
	TArray<FAGX_LidarResultPositionData> Data;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarResultPosition in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarResultPosition_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AssociateWith(
		UPARAM(ref) FAGX_LidarResultPosition& Result, UAGX_LidarSensorComponent* Lidar)
	{
		Result.AssociateWith(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetResult(
		UPARAM(ref) FAGX_LidarResultPosition& Result,
		TArray<FAGX_LidarResultPositionData>& OutResult)
	{
		Result.GetResult(OutResult);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void DebugDrawResult(
		UPARAM(ref) FAGX_LidarResultPosition& Result,
		UAGX_LidarSensorComponent* Lidar)
	{
		Result.DebugDrawResult(Lidar);
	}
};
