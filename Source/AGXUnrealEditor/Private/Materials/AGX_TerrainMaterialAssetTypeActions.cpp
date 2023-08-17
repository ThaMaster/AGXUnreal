// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterialAssetTypeActions.h"

#include "Materials/AGX_TerrainMaterial.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialAssetTypeActions"

FAGX_TerrainMaterialAssetTypeActions::FAGX_TerrainMaterialAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TerrainMaterialAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Terrain Material");
}

const TArray<FText>& FAGX_TerrainMaterialAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("TerrainSubMenu", "Terrain")
	};

	return SubMenus;
}

uint32 FAGX_TerrainMaterialAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TerrainMaterialAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_TerrainMaterialAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines detailed material properties for a terrain.");
}

UClass* FAGX_TerrainMaterialAssetTypeActions::GetSupportedClass() const
{
	return UAGX_TerrainMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
