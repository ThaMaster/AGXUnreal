// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_BaseTerrainParticleRendererComponent.generated.h"

UCLASS(meta=(ShortToolTip="This class is a reusable component for handling of rendering AGX_Terrain particles."))
class AGXUNREAL_API UAGX_BaseTerrainParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_BaseTerrainParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Particle Rendering", meta = (DisplayPriority = "0"))
	bool bEnableParticleRendering = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Particle Rendering")
	void SetEnableParticleRendering(bool bEnabled);

protected:

	AAGX_Terrain* ParentTerrainActor = nullptr;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
	virtual void InitPropertyDispatcher();
#endif

private:
	
	FDelegateHandle DelegateHandle;
	
	bool InitializeParentTerrainActor();
	void BindParticleHandler();
	void UnbindParticleHandler();

	virtual void HandleParticleData(FParticleDataById data);
};
