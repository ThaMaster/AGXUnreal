// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_RayPatternHorizontalSweepTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternHorizontalSweep.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_RayPatternHorizontalSweepTypeActions"

FAGX_RayPatternHorizontalSweepTypeActions::FAGX_RayPatternHorizontalSweepTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_RayPatternHorizontalSweepTypeActions::GetName() const
{
	return LOCTEXT("HorizontalSweepAssetName", "AGX Ray Pattern Horizontal Sweep");
}

uint32 FAGX_RayPatternHorizontalSweepTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_RayPatternHorizontalSweepTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_RayPatternHorizontalSweepTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_RayPatternHorizontalSweepTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("RayPatternAssetDescription", "Holds Lidar Ray Pattern information.");
}

UClass* FAGX_RayPatternHorizontalSweepTypeActions::GetSupportedClass() const
{
	return UAGX_RayPatternHorizontalSweep::StaticClass();
}

#undef LOCTEXT_NAMESPACE
