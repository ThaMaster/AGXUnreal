// Author: VMC Motion Technologies Co., Ltd.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"


/**
 * Asset Type Actions for UAGX_TrackInternalMergePropertiesAsset, customizing its appearance in the Editor menues
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TrackInternalMergePropertiesAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_TrackInternalMergePropertiesAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
