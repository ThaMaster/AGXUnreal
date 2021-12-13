// Copyright 2021, Algoryx Simulation AB.


#include "Materials/AGX_ShapeMaterialAssetTypeActions.h"

#include "Materials/AGX_ShapeMaterialAsset.h"

#define LOCTEXT_NAMESPACE "FAGX_ShapeMaterialAssetTypeActions"

FAGX_ShapeMaterialAssetTypeActions::FAGX_ShapeMaterialAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ShapeMaterialAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Shape Material");
}

uint32 FAGX_ShapeMaterialAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ShapeMaterialAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_ShapeMaterialAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines bulk and surface properties of AGX Shapes.");
}

UClass* FAGX_ShapeMaterialAssetTypeActions::GetSupportedClass() const
{
	return UAGX_ShapeMaterialAsset::StaticClass();
}

#undef LOCTEXT_NAMESPACE
