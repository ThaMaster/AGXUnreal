// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_GeometryContactMergeSplitThresholdsTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_GeometryContactMergeSplitThresholdsBase.h"


#define LOCTEXT_NAMESPACE "FAGX_GeometryContactMergeSplitThresholdsTypeActions"

FAGX_GeometryContactMergeSplitThresholdsTypeActions::FAGX_GeometryContactMergeSplitThresholdsTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_GeometryContactMergeSplitThresholdsTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Geometry Contact Merge Split Thresholds");
}

uint32 FAGX_GeometryContactMergeSplitThresholdsTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_GeometryContactMergeSplitThresholdsTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_GeometryContactMergeSplitThresholdsTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines merge split (AMOR) thresholds for geometry contacts.");
}

UClass* FAGX_GeometryContactMergeSplitThresholdsTypeActions::GetSupportedClass() const
{
	return UAGX_GeometryContactMergeSplitThresholdsBase::StaticClass();
}

#undef LOCTEXT_NAMESPACE