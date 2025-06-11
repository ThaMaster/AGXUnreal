// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainParticleUpsamplerRendererComponent.generated.h"

UCLASS(ClassGroup = "AGX_Terrain_Particle_Rendering", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainParticleUpsamplerRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainParticleUpsamplerRendererComponent();

	/** Whether the native terrain should generate particles or not during shovel interactions. */
	UPROPERTY(EditAnywhere, Category = "AGX Particle Rendering")
	bool bEnableParticleRendering = true;

protected:
private:

};
