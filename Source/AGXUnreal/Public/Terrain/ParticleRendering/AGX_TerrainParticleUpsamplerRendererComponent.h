// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/AGX_BaseTerrainParticleRendererComponent.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainParticleUpsamplerRendererComponent.generated.h"

UCLASS(ClassGroup = "AGX_Terrain_Particle_Rendering", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainParticleUpsamplerRendererComponent
	: public UAGX_BaseTerrainParticleRendererComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainParticleUpsamplerRendererComponent();

	/**
	 * The desired upsampling factor which the renderer will try to achieve.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Particle Upsampling",
		Meta =
			(EditCondition = "bEnableParticleRendering",
			 ClampMin = "1", UIMin = "1", UIMax = "5000"))
	int32 Upsampling = 100;

	/**
	 * Toggle between using the default voxel size or use user defined size.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Particle Upsampling",
		Meta = (EditCondition = "bEnableParticleRendering && bEnableParticleUpsampling"))
	bool bEnableVoxelSize = false;

	/**
	 * The size of how large a singe voxel will be when setting up the data grids.
	 *
	 * Really small voxel size will decrease performance singificantly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Particle Upsampling",
		Meta =
			(EditCondition =
				 "bEnableParticleRendering && bEnableParticleUpsampling && bEnableVoxelSize",
			 UIMin = "10", UIMax = "1000"))
	double VoxelSize = 10.0;

	/**
	 * This controls how quickly the particles are eased in/out when spawned/destroyed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Particle Upsampling",
		Meta =
			(EditCondition = "bEnableParticleRendering && bEnableParticleUpsampling", UIMin = "0.1",
			 UIMax = "1.0"))
	double EaseStepSize = 0.1;

protected:

	virtual void BeginPlay() override;

private:

	const TCHAR* NIAGARA_UPSAMPLING_SYSTEM_PATH = TEXT(
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
		"ParticleUpsampling/PS_ParticleUpsampling_V2.PS_ParticleUpsampling_V2'");

	virtual void HandleParticleData(FParticleDataById data) override;

	void AppendIfActiveVoxel(
		TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius);
	TArray<FIntVector4> GetActiveVoxelsFromSet(TSet<FIntVector> VoxelSet);
};
