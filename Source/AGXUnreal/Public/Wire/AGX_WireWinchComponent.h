#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinch.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

#include "AGX_WireWinchComponent.generated.h"

UCLASS(ClassGroup = "AGX", BlueprintType, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WireWinchComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Winch")
	FAGX_WireWinch WireWinch;
};
