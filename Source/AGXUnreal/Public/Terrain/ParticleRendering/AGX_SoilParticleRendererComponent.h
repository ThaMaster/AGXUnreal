// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CoreTypes.h"

#include "AGX_SoilParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(
	ClassGroup = "AGX_Terrain_Particle_Rendering",
	meta = (BlueprintSpawnableComponent, ShortToolTip = "TODO: WRITE TOOL TIP"))
class AGXUNREAL_API UAGX_SoilParticleRendererComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_SoilParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Particle Rendering")
	bool bEnableParticleRendering = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Particle Rendering")
	void SetEnableParticleRendering(bool bEnabled);

	UPROPERTY(
		EditAnywhere, Category = "AGX Particle Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AGX Particle Rendering")
	UNiagaraComponent* GetParticleSystemComponent();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif

private:

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	AAGX_Terrain* ParentTerrainActor = nullptr;
	UNiagaraComponent* ParticleSystemComponent = nullptr;
	FDelegateHandle DelegateHandle;

	bool InitializeParentTerrainActor();
	bool InitializeNiagaraParticleSystemComponent();
	UNiagaraSystem* FindNiagaraSystemAsset(const TCHAR* AssetPath);

	const TCHAR* NIAGARA_SYSTEM_PATH = TEXT(
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
		"PS_SoilParticleSystem.PS_SoilParticleSystem'");

	UFUNCTION()
	void HandleParticleData(FParticleDataById data);
};
