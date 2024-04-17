// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarResultPosition.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"


bool FAGX_LidarResultPosition::AssociateWith(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	return Lidar->AddResult(*this);
}

void FAGX_LidarResultPosition::DebugDrawResult(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return;

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : Data)
	{
		const FVector Point = Transform.TransformPositionNoScale(Datum.Position);
		DrawDebugPoint(Lidar->GetWorld(), Point, 6.f, FColor::Red, false, 0.12f);
	}
}

bool FAGX_LidarResultPosition::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarResultBarrier* FAGX_LidarResultPosition::GetOrCreateNative()
{
	if (!HasNative())
		NativeBarrier.AllocateNative();

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
	return *this;
}

bool FAGX_LidarResultPosition::operator==(const FAGX_LidarResultPosition& Other) const
{
	return HasNative() && Other.HasNative();
}

void FAGX_LidarResultPosition::GetResult(TArray<FAGX_LidarResultPositionData>& OutResult)
{
	if (HasNative())
		NativeBarrier.GetResult(Data);

	OutResult = Data;
}
