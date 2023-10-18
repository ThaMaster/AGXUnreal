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

inline bool IsTranslatable(EAGX_ShovelFrame ShovelFrame)
{
	switch (ShovelFrame)
	{
		// Even directions are transaltable, even though that doesn't mean anything.
		// Is helpful in order to line the direction vector up with geometry on the shovel.
		case EAGX_ShovelFrame::None:
			return false;
		case EAGX_ShovelFrame::CuttingDirection:
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return true;
	}
}

inline bool IsRotatable(EAGX_ShovelFrame ShovelFrame)
{
	switch (ShovelFrame)
	{
		case EAGX_ShovelFrame::None:
			return false;
		case EAGX_ShovelFrame::CuttingDirection:
			return true;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return false;
	}
}
