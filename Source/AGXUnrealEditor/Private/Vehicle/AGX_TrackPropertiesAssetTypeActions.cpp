// Author: VMC Motion Technologies Co., Ltd.


#include "Vehicle/AGX_TrackPropertiesAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackPropertiesAsset.h"


#define LOCTEXT_NAMESPACE "FAGX_TrackPropertiesAssetTypeActions"

FAGX_TrackPropertiesAssetTypeActions::FAGX_TrackPropertiesAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TrackPropertiesAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Track Properties");
}

uint32 FAGX_TrackPropertiesAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TrackPropertiesAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_TrackPropertiesAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed track properties for AGX Track Component.");
}

UClass* FAGX_TrackPropertiesAssetTypeActions::GetSupportedClass() const
{
	return UAGX_TrackPropertiesAsset::StaticClass();
}

#undef LOCTEXT_NAMESPACE
