// Copyright 2023, Algoryx Simulation AB.

#pragma once


// Unreal Engine includes.
#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"


/**
 * Asset Type Actions for UAGX_ShovelProperties, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_ShovelPropertiesActions : public FAssetTypeActions_Base
{
public:
	FAGX_ShovelPropertiesActions(EAssetTypeCategories::Type InAssetCategory);

	// ~Begin IAssetTypeActions interface.
	FText GetName() const override;
	uint32 GetCategories() override;
	FColor GetTypeColor() const override;
	FText GetAssetDescription(const FAssetData& AssetData) const override;
	UClass* GetSupportedClass() const override;
	// ~End IAssetTypeActions interface.

private:
	EAssetTypeCategories::Type AssetCategory;
};
