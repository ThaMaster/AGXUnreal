// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_DefaultTerrainParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = "AGX_Terrain_Particle_Rendering", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DefaultTerrainParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_DefaultTerrainParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Particle Rendering")
	bool bEnableParticleRendering = true;

	UPROPERTY(
		EditAnywhere, Category = "AGX Particle Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset = nullptr;


	UFUNCTION(BlueprintCallable, Category = "AGX Particle Rendering")
	void SetEnableParticleRendering(bool bEnabled);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Particle related variables.
	UNiagaraComponent* ParticleSystemComponent = nullptr;

	// TODO: Also add the movable terrain component!
	AAGX_Terrain* TerrainActor = nullptr;
	FDelegateHandle DelegateHandle;

	bool InitializeParentTerrainActor();
	bool InitializeParticleSystemComponent();

	void UpdateParticleData(FParticleDataById data);

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;

	void InitPropertyDispatcher();
#endif


};
