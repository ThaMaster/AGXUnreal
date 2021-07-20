#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_WireComponent;
class UAGX_WireWinchComponent;

/// @todo Rename this file to something less specific, but still indicating that it contains Wire
/// and Wire Winch visualization helpers.

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
	/**
	 * Return the transformation that converts the location and rotation of a wire-owned winch from
	 * whatever space they are expressed in to the world coordinate system. For a winch with a body
	 * this is the body's Component Transform. For a winch without a body this is the Wire's
	 * Component Transform.
	 *
	 * @param Wire The Wire that owns the Wire Winch.
	 * @param Winch The Wire Winch to get the transformation for.
	 * @return A transformation that transforms from the Wire Winch's local space to world space.
	 */
	const FTransform& GetOwnedWinchLocalToWorld(
		const UAGX_WireComponent& Wire, const FAGX_WireWinch& Winch);

	/**
	 * Return the transformation that converts the location and rotation of a wire-owned winch from
	 * whatever space they are expressed in to the world coordinate system. For a winch with a body
	 * this is the body's Component Transform. For a winch without a body this is the Wire's
	 * Component Transform.
	 *
	 * @param Wire The Wire that owns the Wire Winch.
	 * @param Side The side of the Wire where the Winch to get the transformation for is.
	 * @return A transformation that transforms from the Wire Winch's local space to world space.
	 */
	const FTransform& GetOwnedWinchLocalToWorld(const UAGX_WireComponent& Wire, EWireSide Side);
	const FTransform& GetWinchLocalToWorld(const UAGX_WireComponent& Wire, EWireSide Side);

	FVector DrawWinch(
		const FAGX_WireWinch& Winch, const FTransform& LocalToWorld,
		HWinchLocationProxy* LocationProxy, HWinchDirectionProxy* DirectionProxy,
		FPrimitiveDrawInterface* PDI);

	FVector DrawWinch(
		const UAGX_WireComponent& Wire, EWireSide Side, const FTransform& LocalToWorld,
		FPrimitiveDrawInterface* PDI);

	FVector DrawWinch(
		const UAGX_WireComponent& Wire, EWireSide Side, FPrimitiveDrawInterface* PDI);

	FVector DrawWinch(const UAGX_WireWinchComponent& Winch, FPrimitiveDrawInterface* PDI);

	bool GetWidgetLocation(
		const FAGX_WireWinch& Winch, const FTransform& WinchToWorld, EWinchSide WinchSide,
		FVector& OutLocation);

	bool GetWidgetLocation(
		const UAGX_WireComponent& WinchOwner, EWireSide WireSide, EWinchSide WinchSide,
		FVector& OutLocation);

	bool GetWidgetLocation(
		const UAGX_WireWinchComponent& Winch, EWinchSide WinchSide, FVector& OutLocation);
}
