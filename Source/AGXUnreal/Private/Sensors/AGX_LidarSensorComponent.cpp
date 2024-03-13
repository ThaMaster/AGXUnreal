// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_LidarSensorComponent::Step()
{
	if (HasNative())
		NativeBarrier.SetTransform(GetComponentTransform());
}

bool UAGX_LidarSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	NativeBarrier.AllocateNative();
	check(NativeBarrier.HasNative());
	return GetNative();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FLidarBarrier* UAGX_LidarSensorComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_LidarSensorComponent::GetResultTest()
{
	NativeBarrier.GetResultTest(GetWorld(), GetComponentTransform());
}

#if WITH_EDITOR
bool UAGX_LidarSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, Range)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
}
#endif
