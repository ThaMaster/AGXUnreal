// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Brick/AGX_BrickAsset.h"
#include "Utilities/AGX_SlateUtilities.h"


#define LOCTEXT_NAMESPACE "FAGX_BrickAssetTypeActions"

FAGX_BrickAssetTypeActions::FAGX_BrickAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_BrickAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Brick Asset");
}

const TArray<FText>& FAGX_BrickAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("BrickSubMenu", "Brick"),
	};

	return SubMenus;
}

uint32 FAGX_BrickAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_BrickAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_BrickAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Brick Asset representing the source Brick file on disk.");
}

UClass* FAGX_BrickAssetTypeActions::GetSupportedClass() const
{
	return UAGX_BrickAsset::StaticClass();
}

bool FAGX_BrickAssetTypeActions::IsImportedAsset() const
{
	return true;
}

void FAGX_BrickAssetTypeActions::GetResolvedSourceFilePaths(
	const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto BrickAsset = CastChecked<UAGX_BrickAsset>(Asset);
		if (BrickAsset->AssetImportData != nullptr)
		{
			BrickAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

#undef LOCTEXT_NAMESPACE
