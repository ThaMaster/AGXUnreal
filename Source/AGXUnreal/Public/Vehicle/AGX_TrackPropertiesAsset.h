// Author: VMC Motion Technologies Co., Ltd.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackPropertiesBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackPropertiesAsset.generated.h"

class UAGX_TrackPropertiesInstance;

/**
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_TrackPropertiesAsset : public UAGX_TrackPropertiesBase
{
	GENERATED_BODY()

public:
	// ~Begin UAGX_TrackPropertiesBase interface.
	virtual UAGX_TrackPropertiesInstance* GetInstance() override;
	virtual UAGX_TrackPropertiesInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
	// ~End UAGX_TrackPropertiesBase interface.

private:
	virtual UAGX_TrackPropertiesAsset* GetAsset() override;

	TWeakObjectPtr<UAGX_TrackPropertiesInstance> Instance;

};
