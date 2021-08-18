#pragma once

struct FAGX_WireWinch;
class UAGX_WireComponent;
class UAGX_WireWinchComponent;

class AGXUNREAL_API FAGX_WireUtilities
{
public:
	static const FTransform& GetVisualizationTransform(
		const UAGX_WireComponent& Owner, const FAGX_WireWinch& Winch);

	static const FTransform& GetVisualizationTransform(
		const UAGX_WireWinchComponent& Owner, const FAGX_WireWinch& Winch);

	static void ComputeSimulationPlacement(
		const UAGX_WireComponent& Owner, FAGX_WireWinch& Winch);

	static void ComputeSimulationPlacement(
		const UAGX_WireWinchComponent& Owner, FAGX_WireWinch& Winch);
};
