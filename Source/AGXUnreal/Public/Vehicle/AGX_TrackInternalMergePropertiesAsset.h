// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergePropertiesBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackInternalMergePropertiesAsset.generated.h"

class UAGX_TrackInternalMergePropertiesInstance;

/**
 * Properties and thresholds for internal merging of nodes in AGX Track Component.
 *
 * Does not own any AGX Native. Instead, it applies its data to the native Track of each Track
 * Component that uses this asset.
 *
 * Asset class, the type used for assets created in the Content Browser. There is also an Instance
 * class that is created as a copy of the asset and that exists for a single Play session only.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_TrackInternalMergePropertiesAsset
	: public UAGX_TrackInternalMergePropertiesBase
{
	GENERATED_BODY()

public:
	// ~Begin UAGX_TrackInternalMergePropertiesBase interface.
	virtual UAGX_TrackInternalMergePropertiesInstance* GetInstance() override;
	virtual UAGX_TrackInternalMergePropertiesInstance* GetOrCreateInstance(
		UWorld* PlayingWorld) override;
	// ~End UAGX_TrackInternalMergePropertiesBase interface.

private:
	virtual UAGX_TrackInternalMergePropertiesAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_TrackInternalMergePropertiesInstance> Instance;
};
