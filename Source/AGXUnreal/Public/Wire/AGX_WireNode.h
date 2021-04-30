#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireNode.generated.h"

USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_WireNode
{
	GENERATED_BODY()

public:
	FAGX_WireNode() = default;
	FAGX_WireNode(const FAGX_WireNode& InOther);
	FAGX_WireNode(FWireNodeBarrier&& InBarrier);

	FAGX_WireNode& operator=(const FAGX_WireNode& InOther);

	FVector GetWorldLocation() const;
	EWireNodeType GetType() const;

private:
	FWireNodeBarrier Barrier;
};
