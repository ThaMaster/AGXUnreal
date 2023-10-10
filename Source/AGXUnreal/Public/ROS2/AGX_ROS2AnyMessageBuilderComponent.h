// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/ROS2AnyMessageBuilderBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

struct FAGX_AgxMsgsAny;

#include "AGX_ROS2AnyMessageBuilderComponent.generated.h"

/**
 * Helper class for serializing custom data types into an agxMsg::Any message that canb e sent via
 * ROS2. The agxMsgs::Any message can then be de-serialized at the receiving side usging the
 * AnyMessageParser.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2AnyMessageBuilderComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2AnyMessageBuilderComponent();

	bool HasNative() const;

	FROS2AnyMessageBuilderBarrier* GetNative();
	const FROS2AnyMessageBuilderBarrier* GetNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	/**
	 * Must be called once each time a new message is to be built, before any of the Write
	 * functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* BeginMessage();

	/** The input is cast to an int8_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt8(int32 data);

	/**
	* Returns the message that has been built by this Component.
	*/
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	FAGX_AgxMsgsAny GetMessage() const;

private:
	FROS2AnyMessageBuilderBarrier NativeBarrier;
};
