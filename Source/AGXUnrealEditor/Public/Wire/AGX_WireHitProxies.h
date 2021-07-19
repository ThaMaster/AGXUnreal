#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_WireComponent;
class UAGX_WireWinchComponent;

/// @todo Rename the Hit Proxies to something wire-specific. Don't want name collisions, one
/// definition rule violations, and undefined behavior.

/**
 * Data associated with clickable node visualization elements.
 */
class HNodeProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HNodeProxy(const UAGX_WireComponent* InWire, int32 InNodeIndex)
		: HComponentVisProxy(InWire, HPP_Wireframe)
		, NodeIndex(InNodeIndex)
	{
	}

	// The index of the node that the visualization that this HNodeProxy is bound to represents.
	int32 NodeIndex;
};

class HWinchLocationProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HWinchLocationProxy(const UAGX_WireComponent* InWire, EWireSide InSide)
		: HComponentVisProxy(InWire, HPP_Wireframe)
		, Side(InSide)
	{
	}

	HWinchLocationProxy(const UAGX_WireWinchComponent* InWinch)
		: HComponentVisProxy(InWinch)
		, Side(EWireSide::None)
	{
	}

	// The side of the wire, begin or end, that this Wire Winch is located.
	EWireSide Side;
};

class HWinchDirectionProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY()

	HWinchDirectionProxy(const UAGX_WireComponent* InWire, EWireSide InSide)
		: HComponentVisProxy(InWire, HPP_Wireframe)
		, Side(InSide)
	{
	}

	HWinchDirectionProxy(const UAGX_WireWinchComponent* InWinch)
		: HComponentVisProxy(InWinch)
		, Side(EWireSide::None)
	{
	}

	// The side of the wire, begin or end, that this Wire Winch is located.
	EWireSide Side;
};

namespace AGX_WireVisualization_helpers
{
	FVector DrawWinch(
		const FAGX_WireWinch& Winch, const FTransform& LocalToWorld,
		HWinchLocationProxy* LocationProxy, HWinchDirectionProxy* DirectionProxy,
		FPrimitiveDrawInterface* PDI);

	FVector DrawWinch(
		const UAGX_WireComponent* WinchOwner, EWireSide Side, const FTransform& LocalToWorld,
		FPrimitiveDrawInterface* PDI);

	FVector DrawWinch(const UAGX_WireWinchComponent* Winch, FPrimitiveDrawInterface* PDI);
}
