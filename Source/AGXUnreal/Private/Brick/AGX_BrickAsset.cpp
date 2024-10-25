// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAsset.h"

// Unreal Engine includes.
#include "EditorFramework/AssetImportData.h"
#include "UObject/AssetRegistryTagsContext.h"

void UAGX_BrickAsset::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	// Not sure exactly what it does. But removing it stops reimport triggering the factory from
	// working.
	if (AssetImportData)
	{
		Context.AddTag(FAssetRegistryTag(
			SourceFileTagName(), AssetImportData->GetSourceData().ToJson(),
			FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(Context);
}
