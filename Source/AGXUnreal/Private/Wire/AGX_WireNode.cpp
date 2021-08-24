#include "Wire/AGX_WireNode.h"

FAGX_WireNode::FAGX_WireNode(const FAGX_WireNode& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_WireNode::FAGX_WireNode(FWireNodeBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

bool FAGX_WireNode::HasNative() const
{
	return Barrier.HasNative();
}

FAGX_WireNode& FAGX_WireNode::operator=(const FAGX_WireNode& InOther)
{
	Barrier = InOther.Barrier;
	return *this;
}

FVector FAGX_WireNode::GetWorldLocation() const
{
	return Barrier.GetWorldLocation();
}

EWireNodeType FAGX_WireNode::GetType() const
{
	return Barrier.GetType();
}

/* Start of Blueprint Function Library. */

FVector UAGX_WireNode_FL::GetWorldLocation(UPARAM(Ref) FAGX_WireNode& Node)
{
	return Node.GetWorldLocation();
}

EWireNodeType UAGX_WireNode_FL::GetType(UPARAM(Ref) FAGX_WireNode& Node)
{
	return Node.GetType();
}
