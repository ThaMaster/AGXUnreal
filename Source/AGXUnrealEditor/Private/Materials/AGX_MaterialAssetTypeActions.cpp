#include "Materials/AGX_MaterialAssetTypeActions.h"

#include "Materials/AGX_ShapeMaterialAsset.h"

#define LOCTEXT_NAMESPACE "FAGX_MaterialAssetTypeActions"

FAGX_MaterialAssetTypeActions::FAGX_MaterialAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_MaterialAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Material");
}

uint32 FAGX_MaterialAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_MaterialAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_MaterialAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines bulk and surface properties of AGX Shapes.");
}

UClass* FAGX_MaterialAssetTypeActions::GetSupportedClass() const
{
	return UAGX_ShapeMaterialAsset::StaticClass();
}

#undef LOCTEXT_NAMESPACE
