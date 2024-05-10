// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPositionIntensity.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"



void FAGX_LidarOutputPositionIntensity::DebugDrawResult(
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

bool FAGX_LidarOutputPositionIntensity::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarOutputBarrier* FAGX_LidarOutputPositionIntensity::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
	}

	return &NativeBarrier;
}

const FLidarOutputBarrier* FAGX_LidarOutputPositionIntensity::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarOutputPositionIntensity& FAGX_LidarOutputPositionIntensity::operator=(const FAGX_LidarOutputPositionIntensity& Other)
{
	FAGX_LidarOutputBase::operator=(Other);
	return *this;
}

bool FAGX_LidarOutputPositionIntensity::operator==(const FAGX_LidarOutputPositionIntensity& Other) const
{
	return FAGX_LidarOutputBase::operator==(Other);
}

void FAGX_LidarOutputPositionIntensity::GetResult(TArray<FAGX_LidarOutputPositionIntensityData>& OutResult)
{
	if (HasNative())
		NativeBarrier.GetResult(Data);

	OutResult = Data;
}
