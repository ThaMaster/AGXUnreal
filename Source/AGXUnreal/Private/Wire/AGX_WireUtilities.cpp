#include "Wire/AGX_WireUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

/*
 * A collection of functions dealing with winch transformations follow.
 *
 * There are a few cases to handle here. The parameters are:
 *  - Owner = {Wire, Winch}
 *  - Native = {false, true}
 *  - Body = {false, true}
 *
 * For each combination we need the transformation to use for visualization rendering.
 *  - Owner=Wire, Native=false, Body=false: Editor placement relative to Wire.
 *  - Owner=Wire, Native=false, Body=true: Editor placement relative to Body.
 *  - Owner=Wire, Native=true, Body=false: Simulation placement relative to World.
 *  - Owner=Wire, Native=true, Body=true: Simulation placement relative to Body.
 *  - Owner=Winch, Native=false, Body=false: Editor placement relative to Winch.
 *  - Owner=Winch, Native=false, Body=true: Editor placement relative to Winch.
 *  - Owner=Winch, Native=true, Body=false: Simulation placement relative to World.
 *  - Owner=Winch, Native=true, Body=true: Simulation placement relative to Body.
 *
 * For each Owner, Body combination we need to go from editor placement to simulation placement.
 *  - Owner=Wire, Body=false: Use Wire transform to convert relative to Wire to relative to World.
 *  - Owner=Wire, Body=true: Both relative to body, no change necessary.
 *  - Owner=Winch, Body=false: Use Winch transform to convert rel. to Winch to rel. to World.
 *  - Owner=Winch, Body=true: Move from relative to Winch to relative to Body.
 */

const FTransform& FAGX_WireUtilities::GetVisualizationTransform(
	const UAGX_WireComponent& Owner, const FAGX_WireWinch& Winch)
{
	const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment();
	const bool bNative = Winch.HasNative();
	const bool bBody = Body != nullptr;
	if (!bNative && !bBody)
	{
		// Owner=Wire, Native=false, Body=false: Editor placement relative to Wire.
		return Owner.GetComponentTransform();
	}
	else if (!bNative && bBody)
	{
		// Owner=Wire, Native=false, Body=true: Editor placement relative to Body.
		return Body->GetComponentTransform();
	}
	else if (bNative && !bBody)
	{
		// Owner=Wire, Native=true, Body=false: Simulation placement relative to World.
		return FTransform::Identity;
	}
	else if (bNative && bBody)
	{
		// Owner=Wire, Native=true, Body=true: Simulation placement relative to Body.
		return Body->GetComponentTransform();
	}

	checkNoEntry();
	return FTransform::Identity;
}

const FTransform& FAGX_WireUtilities::GetVisualizationTransform(
	const UAGX_WireWinchComponent& Owner, const FAGX_WireWinch& Winch)
{
	const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment();
	const bool bNative = Winch.HasNative();
	const bool bBody = Body != nullptr;
	if (!bNative && !bBody)
	{
		// Owner=Winch, Native=false, Body=false: Editor placement relative to Winch.
		return Owner.GetComponentTransform();
	}
	else if (!bNative && bBody)
	{
		// Owner=Winch, Native=false, Body=true: Editor placement relative to Winch.
		return Owner.GetComponentTransform();
	}
	else if (bNative && !bBody)
	{
		// Owner=Winch, Native=true, Body=false: Simulation placement relative to World.
		return FTransform::Identity;
	}
	else if (bNative && bBody)
	{
		// Owner=Winch, Native=true, Body=true: Simulation placement relative to Body.
		return Body->GetComponentTransform();
	}

	checkNoEntry();
	return FTransform::Identity;
}

void FAGX_WireUtilities::ComputeSimulationPlacement(
	const UAGX_WireComponent& Owner, FAGX_WireWinch& Winch)
{
	if (Winch.GetBodyAttachment() == nullptr)
	{
		// Owner=Wire, Body=false: Use Wire transform to convert rel. to Wire to rel. to World.
		Winch.LocationSim = Owner.GetComponentTransform().TransformPosition(Winch.Location);
		Winch.RotationSim = Owner.GetComponentRotation() + Winch.Rotation;
	}
	else
	{
		// Owner=Wire, Body=true: Both relative to body, no change necessary.
		Winch.LocationSim = Winch.Location;
		Winch.RotationSim = Winch.Rotation;
	}
}

void FAGX_WireUtilities::ComputeSimulationPlacement(
	const UAGX_WireWinchComponent& Owner, FAGX_WireWinch& Winch)
{
	const FTransform& WinchTransform = Owner.GetComponentTransform();
	const FRotator& WinchRotation = Owner.GetComponentRotation();
	if (const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment())
	{
		// Owner=Winch, Body=true: Move from relative to Winch to relative to Body.
		const FTransform& BodyTransform = Body->GetComponentTransform();
		const FVector WorldLocation = WinchTransform.TransformPosition(Winch.Location);
		Winch.LocationSim = BodyTransform.InverseTransformPosition(WorldLocation);
		const FQuat& WorldRotation = WinchTransform.TransformRotation(Winch.Rotation.Quaternion());
		Winch.RotationSim = BodyTransform.InverseTransformRotation(WorldRotation).Rotator();
	}
	else
	{
		// Owner=Winch, Body=false: Use Winch transform to convert rel. to Winch to rel. to World.
		Winch.LocationSim = WinchTransform.TransformPosition(Winch.Location);
		Winch.RotationSim = WinchRotation + Winch.Rotation;
	}
}
