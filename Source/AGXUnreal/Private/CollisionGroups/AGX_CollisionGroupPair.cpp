// Copyright 2021, Algoryx Simulation AB.


#include "CollisionGroups/AGX_CollisionGroupPair.h"


bool FAGX_CollisionGroupPair::IsEqual(const FName& GroupA, const FName& GroupB) const
{
	return (Group1.IsEqual(GroupA) && Group2.IsEqual(GroupB)) ||
		(Group1.IsEqual(GroupB) && Group2.IsEqual(GroupA));
}
