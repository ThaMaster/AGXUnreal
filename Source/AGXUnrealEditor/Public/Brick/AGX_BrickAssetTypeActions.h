// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"


class AGXUNREALEDITOR_API FAGX_BrickAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_BrickAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;

	virtual const TArray<FText>& GetSubMenus() const override;

	virtual uint32 GetCategories() override;

	virtual FColor GetTypeColor() const override;

	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;

	virtual UClass* GetSupportedClass() const override;

	virtual bool IsImportedAsset() const override;

	virtual void GetResolvedSourceFilePaths(
		const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
