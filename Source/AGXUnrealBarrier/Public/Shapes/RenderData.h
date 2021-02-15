#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "RenderData.generated.h"

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_RenderData
{
	GENERATED_BODY();

	UPROPERTY()
	uint8 bShouldRender : 1;

	FGuid Guid;
};
