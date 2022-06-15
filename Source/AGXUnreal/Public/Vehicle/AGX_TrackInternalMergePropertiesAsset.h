// Author: VMC Motion Technologies Co., Ltd.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergePropertiesBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackInternalMergePropertiesAsset.generated.h"

class UAGX_TrackInternalMergePropertiesInstance;

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_TrackInternalMergePropertiesAsset : public UAGX_TrackInternalMergePropertiesBase
{
	GENERATED_BODY()

public:
	// ~Begin UAGX_TrackInternalMergePropertiesBase interface.
	virtual UAGX_TrackInternalMergePropertiesInstance* GetInstance() override;
	virtual UAGX_TrackInternalMergePropertiesInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
	// ~End UAGX_TrackInternalMergePropertiesBase interface.

private:
	virtual UAGX_TrackInternalMergePropertiesAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_TrackInternalMergePropertiesInstance> Instance;

};
