// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"

/**
 * Asset Type Actions for UAGX_TrackInternalMergeProperties, customizing its appearance in the
 * Editor menus and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TrackInternalMergePropertiesAssetTypeActions
	: public FAssetTypeActions_Base
{
public:
	explicit FAGX_TrackInternalMergePropertiesAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
