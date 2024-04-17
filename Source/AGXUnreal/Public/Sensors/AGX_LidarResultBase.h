// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarResultBase.generated.h"

class FLidarResultBarrier;
class UAGX_LidarSensorComponent;

USTRUCT(BlueprintType, Meta = (HiddenByDefault))
struct AGXUNREAL_API FAGX_LidarResultBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarResultBase() = default;

	virtual bool AssociateWith(UAGX_LidarSensorComponent* Lidar)
		PURE_VIRTUAL(FAGX_LidarResultBase::AssociateWith, return false;);

	virtual bool HasNative() const PURE_VIRTUAL(FAGX_LidarResultBase::HasNative, return false;);

	virtual FLidarResultBarrier* GetOrCreateNative()
		PURE_VIRTUAL(FAGX_LidarResultBase::GetOrCreateNative, return nullptr;);

	virtual const FLidarResultBarrier* GetNative() const
		PURE_VIRTUAL(FAGX_LidarResultBase::GetNative, return nullptr;);
};
