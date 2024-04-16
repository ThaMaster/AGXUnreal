// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_RayPatternCustomTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_RayPatternCustom.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_RayPatternCustomTypeActions"

FAGX_RayPatternCustomTypeActions::FAGX_RayPatternCustomTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_RayPatternCustomTypeActions::GetName() const
{
	return LOCTEXT("RayPatternAssetName", "AGX Ray Pattern Custom");
}

uint32 FAGX_RayPatternCustomTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_RayPatternCustomTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_RayPatternCustomTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_RayPatternCustomTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("RayPatternAssetDescription", "Holds Lidar Ray Pattern information.");
}

UClass* FAGX_RayPatternCustomTypeActions::GetSupportedClass() const
{
	return UAGX_RayPatternCustom::StaticClass();
}

#undef LOCTEXT_NAMESPACE
