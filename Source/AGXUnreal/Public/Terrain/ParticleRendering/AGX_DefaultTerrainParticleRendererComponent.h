// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/AGX_BaseTerrainParticleRendererComponent.h"

#include "AGX_DefaultTerrainParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = "AGX_Terrain_Particle_Rendering", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DefaultTerrainParticleRendererComponent
	: public UAGX_BaseTerrainParticleRendererComponent
{
	GENERATED_BODY()

public:

	UAGX_DefaultTerrainParticleRendererComponent();

	UPROPERTY(
		EditAnywhere, Category = "AGX Particle Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset = nullptr;

	virtual void BeginPlay() override;

private:

	// Particle related variables.
	UNiagaraComponent* ParticleSystemComponent = nullptr;

	bool InitializeParticleSystemComponent();

	virtual void HandleParticleData(FParticleDataById data) override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostInitProperties() override;
	virtual void InitPropertyDispatcher() override;
#endif


};
