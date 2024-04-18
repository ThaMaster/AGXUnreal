// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarResultPositionIntensity.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"



void FAGX_LidarResultPositionIntensity::DebugDrawResult(
	UAGX_LidarSensorComponent* Lidar, float LifeTime, float Size)
{
	if (Lidar == nullptr)
		return;

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : Data)
	{
		const FVector Point = Transform.TransformPositionNoScale(Datum.Position);
		const uint8 Intensity = static_cast<uint8>(
			FMath::Clamp(Datum.Intensity, 0.0, 1.0) * (double)std::numeric_limits<uint8>::max());

		const FColor Color(Intensity, 0.0, std::numeric_limits<uint8>::max() - Intensity);
		DrawDebugPoint(Lidar->GetWorld(), Point, Size, Color, false, LifeTime);
	}
}

bool FAGX_LidarResultPositionIntensity::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarResultBarrier* FAGX_LidarResultPositionIntensity::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
		PostAllocateNative(&NativeBarrier);
	}

	return &NativeBarrier;
}

const FLidarResultBarrier* FAGX_LidarResultPositionIntensity::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarResultPositionIntensity& FAGX_LidarResultPositionIntensity::operator=(const FAGX_LidarResultPositionIntensity& Other)
{
	bEnableRemovePointsMisses = Other.bEnableRemovePointsMisses;
	bEnableDistanceGaussianNoise = Other.bEnableDistanceGaussianNoise;
	DistanceNoiseSettings = Other.DistanceNoiseSettings;
	
	return *this;
}

bool FAGX_LidarResultPositionIntensity::operator==(const FAGX_LidarResultPositionIntensity& Other) const
{
	return bEnableRemovePointsMisses == Other.bEnableRemovePointsMisses &&
	bEnableDistanceGaussianNoise == Other.bEnableDistanceGaussianNoise &&
	DistanceNoiseSettings == Other.DistanceNoiseSettings &&
	HasNative() && Other.HasNative();
}

void FAGX_LidarResultPositionIntensity::GetResult(TArray<FAGX_LidarResultPositionIntensityData>& OutResult)
{
	if (HasNative())
		NativeBarrier.GetResult(Data);

	OutResult = Data;
}
