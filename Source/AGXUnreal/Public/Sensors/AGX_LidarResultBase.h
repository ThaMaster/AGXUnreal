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

	// Making UAGX_LidarSensorComponent::AddResult Blueprint friendly was not so easy since
	// non-const references becomes out-variables, and pointers to structs are not permitted as
	// input argument.
	bool AssociateWith(UAGX_LidarSensorComponent* Lidar);

	virtual bool HasNative() const PURE_VIRTUAL(FAGX_LidarResultBase::HasNative, return false;);

	virtual FLidarResultBarrier* GetOrCreateNative()
		PURE_VIRTUAL(FAGX_LidarResultBase::GetOrCreateNative, return nullptr;);

	virtual const FLidarResultBarrier* GetNative() const
		PURE_VIRTUAL(FAGX_LidarResultBase::GetNative, return nullptr;);
};
