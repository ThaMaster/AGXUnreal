// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"

#include "AGX_ParticleUpsamplingInterface.generated.h"

USTRUCT()
struct FCoarseParticle
{
	GENERATED_BODY();

	FVector4f PositionAndRadius;
	FVector4f VelocityAndMass;
};

USTRUCT()
struct FVoxelEntry
{
	GENERATED_BODY();

	FIntVector4 IndexAndRoom;
	FVector4f VelocityAndMass;
	FVector4f MaxBounds;
	FVector4f MinBounds;
};

UCLASS(EditInlineNew, Category = "Data Interface", CollapseCategories, meta = (DisplayName = "Particle Upsampling Interface"))
class AGXUNREAL_API UAGX_ParticleUpsamplingInterface : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		// Particle Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FCoarseParticle>, CoarseParticles)
		SHADER_PARAMETER(int,									NumCoarseParticles)
		SHADER_PARAMETER(float,									FineParticleMass)
		SHADER_PARAMETER(float,									FineParticleRadius)
		SHADER_PARAMETER(float,									NominalRadius)

		// HashTable Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FIntVector4>,		ActiveVoxelIndices)
		SHADER_PARAMETER(int,									NumActiveVoxels)

		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVoxelEntry>,	HashTable)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>,			HTOccupancy)

		SHADER_PARAMETER(int,									TableSize)
		SHADER_PARAMETER(float,									VoxelSize)

		// Other Variables
		SHADER_PARAMETER(int,									Time)
		SHADER_PARAMETER(float,									AnimationSpeed)
	END_SHADER_PARAMETER_STRUCT()

public:
protected:
private:
	const static float PACKING_RATIO;
};
