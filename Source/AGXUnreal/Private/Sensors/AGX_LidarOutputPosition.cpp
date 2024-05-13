// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPosition.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"

void FAGX_LidarOutputPosition::DebugDrawData(
	const TArray<FAGX_LidarOutputPositionData>& InData, UAGX_LidarSensorComponent* Lidar,
	float LifeTime, float Size, FColor Color)
{
	if (Lidar == nullptr)
		return;

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : InData)
	{
		const FVector Point = Transform.TransformPositionNoScale(Datum.Position);
		DrawDebugPoint(Lidar->GetWorld(), Point, Size, Color, false, LifeTime);
	}
}

bool FAGX_LidarOutputPosition::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarOutputBarrier* FAGX_LidarOutputPosition::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
	}

	return &NativeBarrier;
}

const FLidarOutputBarrier* FAGX_LidarOutputPosition::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarOutputPosition& FAGX_LidarOutputPosition::operator=(const FAGX_LidarOutputPosition& Other)
{
	FAGX_LidarOutputBase::operator=(Other);
	return *this;
}

bool FAGX_LidarOutputPosition::operator==(const FAGX_LidarOutputPosition& Other) const
{
	return FAGX_LidarOutputBase::operator==(Other);
}

void FAGX_LidarOutputPosition::GetData(TArray<FAGX_LidarOutputPositionData>& OutData)
{
	if (HasNative())
		NativeBarrier.GetData(Data);

	OutData = Data;
}
