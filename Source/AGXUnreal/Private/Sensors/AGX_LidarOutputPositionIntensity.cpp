// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPositionIntensity.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"


void FAGX_LidarOutputPositionIntensity::DebugDrawData(
	const TArray<FAGX_LidarOutputPositionIntensityData>& InData, UAGX_LidarSensorComponent* Lidar,
	float LifeTime, float BaseSize)
{
	if (Lidar == nullptr)
		return;

	if (!Lidar->bEnableRendering)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPositionIntensity::DebugDrawData called but the given Lidar does "
				 "not have bEnableRendering set to true. Doing nothing."));
		return;
	}

	UNiagaraComponent* Nc = Lidar->GetSpawnedNiagaraSystemComponent();
	if (Nc == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPositionIntensity::DebugDrawData called but the given Lidar does "
				 "not have a spawned Niagara Component. Doing nothing."));
		return;
	}

	RenderPositions.SetNum(0, false);
	RenderColors.SetNum(0, false);

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : InData)
	{
		const FVector Point = Transform.TransformPositionNoScale(Datum.Position);
		const uint8 Intensity = static_cast<uint8>(
			FMath::Clamp(Datum.Intensity, 0.0, 1.0) * (double) std::numeric_limits<uint8>::max());

		const FLinearColor Color = FLinearColor::FromSRGBColor(
			FColor(Intensity, 0.0, std::numeric_limits<uint8>::max() - Intensity));

		RenderPositions.Add(Point);
		RenderColors.Add(Color);
	}

	Nc->SetNiagaraVariableInt("User.NumPoints", InData.Num());
	Nc->SetNiagaraVariableFloat("User.Lifetime", LifeTime);
	Nc->SetNiagaraVariableFloat("User.ZeroDistanceSize", BaseSize);

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
		Nc, "Positions", RenderPositions);

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayColor(Nc, "Colors", RenderColors);
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

FAGX_LidarOutputPositionIntensity& FAGX_LidarOutputPositionIntensity::operator=(
	const FAGX_LidarOutputPositionIntensity& Other)
{
	return *this;
}

bool FAGX_LidarOutputPositionIntensity::operator==(
	const FAGX_LidarOutputPositionIntensity& Other) const
{
	return FAGX_LidarOutputBase::operator==(Other);
}

void FAGX_LidarOutputPositionIntensity::GetData(
	TArray<FAGX_LidarOutputPositionIntensityData>& OutData)
{
	if (HasNative())
		NativeBarrier.GetData(Data);

	OutData = Data;
}
