// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2PublisherBarrier.h"
#include "ROS2/AGX_ROS2Enums.h"
#include "ROS2/AGX_ROS2Qos.h"

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

	/**
	 * Struct containing Quality of Service (QOS) settings.
	 * Reliability, durability, history and historyDepth are supported.
	 * By default the QOS settings are the same as in ROS2, i.e.
	 * Reliability "reliable", Durability "volatile", Durability "keep last" and history depth 10.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	FAGX_ROS2Qos Qos;

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Float32"))
	void SendStdMsgsFloat32(const FAGX_StdMsgsFloat32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Int32"))
	void SendStdMsgsInt32(const FAGX_StdMsgsInt32& Message, const FString& Topic);

private:
	TMap<FString, FROS2PublisherBarrier> NativeBarriers;

	FROS2PublisherBarrier* GetOrCreateBarrier(EAGX_ROS2MessageType Type, const FString& Topic);

	// ~Begin UActorComponent interface.
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// ~Begin UActorComponent interface.
};
