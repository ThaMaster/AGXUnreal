// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPosition.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"


void FAGX_LidarOutputPosition::Render(
	const TArray<FAGX_LidarOutputPositionData>& InData, UAGX_LidarSensorComponent* Lidar,
	float LifeTime, float BaseSize)
{
	if (Lidar == nullptr)
		return;

	if (!Lidar->bEnableRendering)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPosition::Render called but the given Lidar does "
				 "not have bEnableRendering set to true. Doing nothing."));
		return;
	}

	UNiagaraComponent* Nc = Lidar->GetSpawnedNiagaraSystemComponent();
	if (Nc == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPosition::Render called but the given Lidar does "
				 "not have a spawned Niagara Component. Doing nothing."));
		return;
	}

	// Lidar in AGX Dynamics has output data left in the buffers even after being disabled.
	// Therefore, we check for this explicitly to not render old data after Lidar->setEnable(false).
	const TArray<FAGX_LidarOutputPositionData> NoData;
	const TArray<FAGX_LidarOutputPositionData>& DataToRender =
		Lidar->GetEnabled() ? InData : NoData;

	RenderPositions.SetNum(0, false);

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : DataToRender)
	{
		RenderPositions.Add(Transform.TransformPositionNoScale(Datum.Position));
	}

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Nc->SetNiagaraVariableInt("User.NumPoints", DataToRender.Num());
	Nc->SetNiagaraVariableFloat("User.Lifetime", LifeTime);
	Nc->SetNiagaraVariableFloat("User.ZeroDistanceSize", BaseSize);
#else
	Nc->SetVariableInt(FName("User.NumPoints"), DataToRender.Num());
	Nc->SetVariableFloat(FName("User.Lifetime"), LifeTime);
	Nc->SetVariableFloat(FName("User.ZeroDistanceSize"), BaseSize);
#endif

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
		Nc, "Positions", RenderPositions);
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
