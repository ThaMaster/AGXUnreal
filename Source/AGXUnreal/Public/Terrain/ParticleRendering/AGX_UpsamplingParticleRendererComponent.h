// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CoreTypes.h"

#include "AGX_UpsamplingParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(
	ClassGroup = "AGX_Terrain_Particle_Rendering",
	meta = (BlueprintSpawnableComponent, ShortToolTip = "TODO: WRITE TOOL TIP"))
class AGXUNREAL_API UAGX_UpsamplingParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_UpsamplingParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	bool bEnableParticleRendering = true;

	UPROPERTY(EditAnywhere, Category = "AGX Particle Rendering")
	UNiagaraSystem* ParticleSystemAsset;

	/**
	 * The desired upsampling factor which the renderer will try to achieve.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering",
			 ClampMin = "1", UIMin = "1", UIMax = "5000"))
	int32 Upsampling = 100;

	/**
	 * Toggle between using the default voxel size or use user defined size.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	bool bEnableVoxelSize = false;

	/**
	 * The size of how large a singe voxel will be when setting up the data grids.
	 *
	 * Really small voxel size will decrease performance singificantly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition =
				 "bEnableParticleRendering && bEnableVoxelSize",
			 UIMin = "10", UIMax = "1000"))
	double VoxelSize = 10.0;

	/**
	 * This controls how quickly the particles are eased in/out when spawned/destroyed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering", UIMin = "0.1",
			 UIMax = "1.0"))
	double EaseStepSize = 0.1;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool SetEnableParticleRendering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableParticleRendering();

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	UNiagaraComponent* GetParticleSystemComponent();
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 SetUpsampling(int32 InUpsampling);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 GetUpsampling();

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool SetEnableVoxelSize(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableVoxelSize();
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double SetVoxelSize(double InVoxelSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetVoxelSize();

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double SetEaseStepSize(double InEaseStepSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetEaseStepSize();
	
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
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/UpsamplingParticleSystem"
		"/PS_ParticleUpsamplingSystem.PS_ParticleUpsamplingSystem'");

	UFUNCTION()
	void HandleParticleData(FDelegateParticleData data);

	void AppendIfActiveVoxel(
		TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius);
	TArray<FIntVector4> GetActiveVoxelsFromSet(TSet<FIntVector> VoxelSet);
};