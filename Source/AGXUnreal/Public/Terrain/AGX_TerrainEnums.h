// Copyright 2023, Algoryx Simulation AB.

#pragma once

/**
 * All frames that a Shovel keeps track of.
 */
UENUM()
enum class EAGX_ShovelFrame : uint8
{
	None,
	TopEdgeBegin,
	TopEdgeEnd,
	CuttingEdgeBegin,
	CuttingEdgeEnd,
	CuttingDirection
};
