// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BrickAsset.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_BrickAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Brick")
	int Dummy;

#if WITH_EDITORONLY_DATA

	/** The file this data table was imported from, may be empty */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSource)
	TObjectPtr<class UAssetImportData> AssetImportData;

	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
#endif
};
