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
class UAGX_ParticleUpsamplingDI;

UCLASS(
	ClassGroup = "AGX_Terrain_Particle_Rendering",
	meta = (BlueprintSpawnableComponent, ShortToolTip = "Uses the particle data from AGX_Terrain to upsample the rendered particles, significantly increasing particle count."))
class AGXUNREAL_API UAGX_UpsamplingParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_UpsamplingParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * The desired upsampling factor which the renderer will try to achieve.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (ClampMin = "1", UIMin = "1", UIMax = "5000"))
	int32 Upsampling = 100;

	/**
	 * Toggle between using the default voxel size or use user defined size.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	bool bOverrideVoxelSize = false;

	/**
	 * The size of how large a singe voxel will be when setting up the data grids.
	 *
	 * Really small voxel size will decrease performance singificantly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (EditCondition = "bOverrideVoxelSize", ClampMin=10, UIMin = "10", 
			UIMax = "1000"))
	float VoxelSize = 10.0;

	/**
	 * This controls how quickly the particles are eased in/out when spawned/destroyed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (UIMin = "0.1", UIMax = "1.0"))
	double EaseStepSize = 0.1;

	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	UNiagaraSystem* ParticleSystemAsset;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetEnableParticleRendering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableParticleRendering() const;

	/**
	 * If a Particle System Component has been spawned by the Renderer Component, this function will return it.
	 * Returns nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	UNiagaraComponent* GetSpawnedParticleSystemComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetUpsampling(int32 InUpsampling);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 GetUpsampling() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetOverrideVoxelSize(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetOverrideVoxelSize() const;
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetVoxelSize(double InVoxelSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetVoxelSize() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetEaseStepSize(double InEaseStepSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetEaseStepSize() const;
	
protected:

	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	// ~End UActorComponent interface.

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	// ~End UObject interface.


private:

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	UNiagaraComponent* ParticleSystemComponent = nullptr;
	UAGX_ParticleUpsamplingDI* UpsamplingDataInterface = nullptr;

	float ElementSize = 0;

	UFUNCTION()
	void HandleParticleData(FDelegateParticleData& data);

	/** 
	 * Appends the voxel indices that a coarse particle intersects with to the given array. 
	 */
	void AppendIfActiveVoxel(
		TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius, float SizeOfVoxel);

	/** 
	 * Converts the active voxels from the set to a TArray 
	 */
	TArray<FIntVector4> GetActiveVoxelsFromSet(TSet<FIntVector> VoxelSet);
};