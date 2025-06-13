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
	ClassGroup = "AGX_Terrain_Particle_Rendering", Blueprintable,
	meta = (BlueprintSpawnableComponent, ShortToolTip = "TODO: WRITE TOOL TIP"))
class AGXUNREAL_API UAGX_SoilParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_SoilParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Soil Particle Rendering")
	bool bEnableParticleRendering = true;

	UPROPERTY(EditAnywhere, Category = "AGX Soil Particle Rendering")
	UNiagaraSystem* ParticleSystemAsset; // THIS IS GRAY FOR SOME REASON?!?????

	UFUNCTION(BlueprintCallable, Category = "AGX Soil Particle Rendering")
	bool SetEnableParticleRendering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Soil Particle Rendering")
	bool GetEnableParticleRendering();

	UFUNCTION(BlueprintCallable, Category = "AGX Soil Particle Rendering")
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
	void AssignDefaultNiagaraAsset(auto*& AssetRefProperty, const TCHAR* AssetPath);

	UFUNCTION()
	void HandleParticleData(FParticleDataById data);
};
