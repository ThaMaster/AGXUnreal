// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSurfaceMaterialTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSurfaceMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSurfaceMaterialTypeActions"

FAGX_LidarSurfaceMaterialTypeActions::FAGX_LidarSurfaceMaterialTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LidarSurfaceMaterialTypeActions::GetName() const
{
	return LOCTEXT("LidarSurfaceMaterialAssetName", "AGX Lidar Surface Material");
}

uint32 FAGX_LidarSurfaceMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LidarSurfaceMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LidarSurfaceMaterialTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LidarSurfaceMaterialTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LidarSurfaceMaterialAssetDesc", "Holds Lidar Surface Material information.");
}

UClass* FAGX_LidarSurfaceMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_LidarSurfaceMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
