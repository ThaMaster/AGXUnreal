// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"

struct FCoarseParticle
{
	FVector4f PositionAndRadius;
	FVector4f VelocityAndMass;
};

struct FVoxelEntry
{
	FIntVector4 IndexAndRoom;
	FVector4f VelocityAndMass;
	FVector4f MaxBounds;
	FVector4f MinBounds;
};


struct FPUBuffers : public FRenderResource
{
};

struct FPUArrays
{

};

/** 
 * Struct containing all the data which is needed for the Particle
 * Upsampling Data Interface.
 */
struct AGX_ParticleUpsamplingData
{
	FPUBuffers* PUBuffers = nullptr;
	FPUArrays* PUArrays = nullptr;
};
