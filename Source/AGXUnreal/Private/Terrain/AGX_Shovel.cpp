// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_Shovel.h"

#include "Terrain/ShovelBarrier.h"

void FAGX_Shovel::UpdateNativeShovelProperties(
	FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel)
{
	ShovelBarrier.SetVerticalBladeSoilMergeDistance(Shovel.VerticalBladeSoilMergeDistance);
	ShovelBarrier.SetNoMergeExtensionDistance(Shovel.NoMergeExtensionDistance);
	ShovelBarrier.SetPenetrationForceScaling(Shovel.PenetrationForceScaling);
	ShovelBarrier.SetAlwaysRemoveShovelContacts(Shovel.AlwaysRemoveShovelContacts);

	auto SetExcavationSettings =
		[&ShovelBarrier](EAGX_ExcavationMode Mode, const FAGX_ShovelExcavationSettings& Settings)
	{
		ShovelBarrier.SetExcavationSettingsEnabled(Mode, Settings.bEnabled);
		ShovelBarrier.SetExcavationSettingsEnableCreateDynamicMass(
			Mode, Settings.bEnableCreateDynamicMass);
		ShovelBarrier.SetExcavationSettingsEnableForceFeedback(Mode, Settings.bEnableForceFeedback);
	};

	SetExcavationSettings(EAGX_ExcavationMode::Primary, Shovel.PrimaryExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformBack, Shovel.DeformBackExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformRight, Shovel.DeformRightExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformLeft, Shovel.DeformLeftExcavationSettings);
}
