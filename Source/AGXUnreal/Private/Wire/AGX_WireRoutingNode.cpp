// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireRoutingNode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

void FWireRoutingNode::SetBody(UAGX_RigidBodyComponent* Body)
{
	if (Body == nullptr)
	{
		RigidBody.OwningActor = nullptr;
		RigidBody.Name = NAME_None;
		return;
	}

	RigidBody.OwningActor = Body->GetOwner();
	RigidBody.Name = Body->GetFName();
}

void UAGX_WireRouteNode_FL::SetBody(
	UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
{
	WireNode.SetBody(Body);
}
