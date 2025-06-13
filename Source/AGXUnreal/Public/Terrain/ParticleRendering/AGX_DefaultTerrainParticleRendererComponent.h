// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/AGX_ParticleRendererComponentBase.h"

#include "AGX_DefaultTerrainParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = "AGX_Terrain_Particle_Rendering", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DefaultTerrainParticleRendererComponent
	: public UAGX_ParticleRendererComponentBase
{
	GENERATED_BODY()

public:

	UAGX_DefaultTerrainParticleRendererComponent();

protected:

	virtual void BeginPlay() override;

private:

	const TCHAR* NIAGARA_SYSTEM_PATH = TEXT(
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
		"PS_SoilParticleSystem.PS_SoilParticleSystem'");

	virtual void HandleParticleData(FParticleDataById data) override;
};
