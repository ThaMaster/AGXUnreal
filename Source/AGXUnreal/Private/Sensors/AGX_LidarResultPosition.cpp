// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarResultPosition.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"

void FAGX_LidarResultPosition::DebugDrawResult(
	UAGX_LidarSensorComponent* Lidar, float LifeTime, float Size, FColor Color)
{
	if (Lidar == nullptr)
		return;

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : Data)
	{
		const FVector Point = Transform.TransformPositionNoScale(Datum.Position);
		DrawDebugPoint(Lidar->GetWorld(), Point, Size, Color, false, LifeTime);
	}
}

bool FAGX_LidarResultPosition::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarResultBarrier* FAGX_LidarResultPosition::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
		PostAllocateNative(&NativeBarrier);
	}

	return &NativeBarrier;
}

const FLidarResultBarrier* FAGX_LidarResultPosition::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarResultPosition& FAGX_LidarResultPosition::operator=(const FAGX_LidarResultPosition& Other)
{
	bEnableRemovePointsMisses = Other.bEnableRemovePointsMisses;
	bEnableDistanceGaussianNoise = Other.bEnableDistanceGaussianNoise;
	DistanceNoiseSettings = Other.DistanceNoiseSettings;

	return *this;
}

bool FAGX_LidarResultPosition::operator==(const FAGX_LidarResultPosition& Other) const
{
	return bEnableRemovePointsMisses == Other.bEnableRemovePointsMisses &&
		   bEnableDistanceGaussianNoise == Other.bEnableDistanceGaussianNoise &&
		   DistanceNoiseSettings == Other.DistanceNoiseSettings && HasNative() && Other.HasNative();
}

void FAGX_LidarResultPosition::GetResult(TArray<FAGX_LidarResultPositionData>& OutResult)
{
	if (HasNative())
		NativeBarrier.GetResult(Data);

	OutResult = Data;
}
