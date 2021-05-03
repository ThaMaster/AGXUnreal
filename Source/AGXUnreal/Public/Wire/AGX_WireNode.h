#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WireNode.generated.h"

/**
 * A FAGX_WireNode is a handle to an AGX Dynamics Wire Node. It does not own the underlying node,
 * the Wire does that, and since wire nodes are created and destroyed throughout the simulation it
 * is safest to re-fetch the nodes each frame through the Wire Component. Some nodes, such as Eye
 * and Body Fixed, are persistent and guaranteed to not be removed by the wire resolution updates.
 */
USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_WireNode
{
	GENERATED_BODY()

public:
	FAGX_WireNode() = default;
	FAGX_WireNode(const FAGX_WireNode& InOther);
	FAGX_WireNode(FWireNodeBarrier&& InBarrier);

	FAGX_WireNode& operator=(const FAGX_WireNode& InOther);

	/// @return The world location of this node.
	FVector GetWorldLocation() const;

	/// @return The type of this node.
	EWireNodeType GetType() const;

private:
	FWireNodeBarrier Barrier;
};
