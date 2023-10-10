// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/ROS2AnyMessageParserBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

struct FAGX_AgxMsgsAny;

#include "AGX_ROS2AnyMessageParserComponent.generated.h"

/**
 * Helper class for serializing custom data types into an agxMsg::Any message that canb e sent via
 * ROS2. The agxMsgs::Any message can then be de-serialized at the receiving side usging the
 * AnyMessageParser.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2AnyMessageParserComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2AnyMessageParserComponent();

	bool HasNative() const;

	FROS2AnyMessageParserBarrier* GetNative();
	const FROS2AnyMessageParserBarrier* GetNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	/**
	 * Must be called once each time a new message is to be parsed, before any of the Read
	 * functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageParserComponent* BeginParse(const FAGX_AgxMsgsAny& Message);

	/** The result is cast to an int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int32 ReadInt8();

private:
	FROS2AnyMessageParserBarrier NativeBarrier;
};
