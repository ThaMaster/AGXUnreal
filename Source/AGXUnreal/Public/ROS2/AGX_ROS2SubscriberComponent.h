// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2SubscriberBarrier.h"
#include "ROS2/AGX_ROS2Enums.h"
#include "ROS2/AGX_ROS2Qos.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_ROS2SubscriberComponent.generated.h"

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2SubscriberComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2SubscriberComponent();

	/**
	 * Struct containing Quality of Service (QOS) settings.
	 * Reliability, durability, history and historyDepth are supported.
	 * By default the QOS settings are the same as in ROS2, i.e.
	 * Reliability "reliable", Durability "volatile", Durability "keep last" and history depth 10.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	FAGX_ROS2Qos Qos;

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Float32"))
	bool ReceiveStdMsgFloat32(FAGX_StdMsgsFloat32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Int32"))
	bool ReceiveStdMsgsInt32(FAGX_StdMsgsInt32& OutMessage, const FString& Topic);

private:
	TMap<FString, FROS2SubscriberBarrier> NativeBarriers;

	FROS2SubscriberBarrier* GetOrCreateBarrier(EAGX_ROS2MessageType Type, const FString& Topic);

	// ~Begin UActorComponent interface.
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// ~Begin UActorComponent interface.
};
