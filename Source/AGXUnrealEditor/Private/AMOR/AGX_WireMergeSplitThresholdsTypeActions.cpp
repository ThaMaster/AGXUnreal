// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholdsTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_WireMergeSplitThresholdsBase.h"


#define LOCTEXT_NAMESPACE "FAGX_WireMergeSplitThresholdsTypeActions"

FAGX_WireMergeSplitThresholdsTypeActions::FAGX_WireMergeSplitThresholdsTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_WireMergeSplitThresholdsTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Wire Merge Split Thresholds");
}

uint32 FAGX_WireMergeSplitThresholdsTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_WireMergeSplitThresholdsTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_WireMergeSplitThresholdsTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines merge split (AMOR) thresholds for Wires.");
}

UClass* FAGX_WireMergeSplitThresholdsTypeActions::GetSupportedClass() const
{
	return UAGX_WireMergeSplitThresholdsBase::StaticClass();
}

#undef LOCTEXT_NAMESPACE