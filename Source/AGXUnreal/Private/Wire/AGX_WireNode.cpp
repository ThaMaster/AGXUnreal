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
