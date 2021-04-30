#include "AGX_WireNode.h"

FAGX_WireNode::FAGX_WireNode(const FAGX_WireNode& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_WireNode::FAGX_WireNode(FWireNodeBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
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
