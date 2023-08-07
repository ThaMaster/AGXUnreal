// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AGX_ShovelComponent.generated.h"


UCLASS(ClassGroup="AGX_Terrain", meta=(BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ShovelComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_ShovelComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
