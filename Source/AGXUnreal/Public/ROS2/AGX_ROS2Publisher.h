// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2PublisherBarrier.h"
#include "ROS2/AGX_ROS2Enums.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_ROS2Publisher.generated.h"


UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2Publisher : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2Publisher();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
	void SendStdMsgsFloat32(const FAGX_StdMsgsFloat32& Msg, const FString& Topic);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
	void SendStdMsgsInt32(const FAGX_StdMsgsInt32& Msg, const FString& Topic);

private:
	TMap<FString, FROS2PublisherBarrier> NativeBarriers;

	FROS2PublisherBarrier* GetOrCreateBarrier(EAGX_ROS2MessageType Type, const FString& Topic);
};
