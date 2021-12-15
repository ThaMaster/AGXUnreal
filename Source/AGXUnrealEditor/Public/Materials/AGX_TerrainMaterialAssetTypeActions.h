// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"

/**
 * Asset Type Actions for UAGX_TerrainMaterialAsset, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TerrainMaterialAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_TerrainMaterialAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
