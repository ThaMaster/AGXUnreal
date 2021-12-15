// Copyright 2021, Algoryx Simulation AB.


#include "Terrain/AGX_Shovel.h"

#include "Terrain/ShovelBarrier.h"

void FAGX_Shovel::UpdateNativeShovelProperties(
	FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel)
{
	ShovelBarrier.SetVerticalBladeSoilMergeDistance(Shovel.VerticalBladeSoilMergeDistance);
	ShovelBarrier.SetNoMergeExtensionDistance(Shovel.NoMergeExtensionDistance);
	ShovelBarrier.SetPenetrationForceScaling(Shovel.PenetrationForceScaling);
	ShovelBarrier.SetAlwaysRemoveShovelContacts(Shovel.AlwaysRemoveShovelContacts);
}
