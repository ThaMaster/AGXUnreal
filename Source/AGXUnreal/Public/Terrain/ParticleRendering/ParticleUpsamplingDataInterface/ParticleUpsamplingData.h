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
	const uint32 INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;
	const uint32 INITIAL_VOXEL_BUFFER_SIZE = 1024;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;
	virtual FString GetFriendlyName() const override;

	template <typename T>
	FShaderResourceViewRHIRef InitSRVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);
	template <typename T>
	FUnorderedAccessViewRHIRef InitUAVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	void UpdateCoarseParticleBuffers(
		FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData,
		uint32 NewElementCount, bool NeedsResize);
	void UpdateHashTableBuffers(
		FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices,
		uint32 NewElementCount, bool NeedsResize);

	// SRV Buffers
	FShaderResourceViewRHIRef CoarseParticleBufferRef;
	FShaderResourceViewRHIRef ActiveVoxelIndicesBufferRef;

	// RW Buffers
	FUnorderedAccessViewRHIRef HashTableBufferRef;
	FUnorderedAccessViewRHIRef HashTableOccupancyBufferRef;
};

struct FPUArrays
{
	const uint32 INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;
	const uint32 INITIAL_VOXEL_BUFFER_SIZE = 1024;

	FPUArrays();
	void CopyFrom(const FPUArrays* Other);
	void SetNewTime(int NewTime);

	TArray<FCoarseParticle> CoarseParticles;
	TArray<FIntVector4> ActiveVoxelIndices;

	uint32 NumElementsInActiveVoxelBuffer = INITIAL_VOXEL_BUFFER_SIZE;
	uint32 NumElementsInCoarseParticleBuffer = INITIAL_COARSE_PARTICLE_BUFFER_SIZE;

	int Time = 0;
	float VoxelSize = 0;
	float FineParticleMass = 0;
	float FineParticleRadius = 0;
	float EaseStepSize = 0;
	float NominalRadius = 0;
	int TableSize = 0;

	bool NeedsCPResize = false;
	bool NeedsVoxelResize = false;
};

/** 
 * Struct containing all the data which is needed for the Particle
 * Upsampling Data Interface.
 */
struct FParticleUpsamplingData
{
	void Init(FNiagaraSystemInstance* SystemInstance);
	void Update(FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherData);
	void Release();

	FPUBuffers* PUBuffers = nullptr;
	FPUArrays* PUArrays = nullptr;
};
