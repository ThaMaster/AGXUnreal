// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialAssetTypeActions.h"

#include "Materials/AGX_ShapeMaterial.h"

#define LOCTEXT_NAMESPACE "FAGX_ShapeMaterialTypeActions"

FAGX_ShapeMaterialTypeActions::FAGX_ShapeMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ShapeMaterialTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Shape Material");
}

uint32 FAGX_ShapeMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ShapeMaterialTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_ShapeMaterialTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines bulk and surface properties of AGX Shapes.");
}

UClass* FAGX_ShapeMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_ShapeMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
