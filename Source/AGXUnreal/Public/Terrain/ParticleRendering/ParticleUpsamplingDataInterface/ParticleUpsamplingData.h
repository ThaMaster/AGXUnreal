// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "NiagaraCommon.h"

/**
 * Struct to store data from a single coarse particle.
 */ 
struct FCoarseParticle
{
	/**
	 * The position stored at XYZ, the particle radius stored at W.
	 */
	FVector4f PositionAndRadius;

	/**
	 * The velocity stored at XYZ, the particle mass stored at W.
	 */
	FVector4f VelocityAndMass;
};

/**
 * Struct to store data contained in a voxel of the voxel grid.
 */
struct FVoxelEntry
{
	/**
	 * The voxel index stored at XYZ, the voxel room stored at W.
	 */
	FIntVector4 IndexAndRoom;

	/**
	 * The voxel velocity stored at XYZ, the voxel mass stored at W.
	 */
	FVector4f VelocityAndMass;

	/**
	 * The max coordinates of the bounding box surrounding coarse particles
	 * in this voxel stored at XYZ, W is empty.
	 */
	FVector4f MaxBounds;

	/**
	 * The min coordinates of the bounding box surrounding coarse particles 
	 * in this voxel stored at XYZ, W is empty.
	 */
	FVector4f MinBounds;
};

struct FPUBuffers : public FRenderResource
{
	FPUBuffers()
		: NumAllocatedElementsInCoarseParticleBuffer(0)
		, NumAllocatedElementsInActiveVoxelBuffer(0)
	{
	}

	FPUBuffers(uint32 InitialCPElements, uint32 InitialActiveVoxelElements)
		: NumAllocatedElementsInCoarseParticleBuffer(InitialCPElements) 
		, NumAllocatedElementsInActiveVoxelBuffer(InitialActiveVoxelElements)
	{
	}
	
	// ~Begin FRenderResource interface.

	/**
	 * Initialize the different buffers used in this render resource. 
	 */
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	/** 
	 * Release all buffers used in this render resource.  
	 */
	virtual void ReleaseRHI() override;
	virtual FString GetFriendlyName() const override { return TEXT("Particle Upsampling Render Resources") ;};

	// ~End FRenderResource interface.

	/** 
	 * Function for initializing a new Read-only buffer on the GPU 
	 */
	template <typename T>
	FShaderResourceViewRHIRef InitSRVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	/** 
	 * Function for initializing a new Read/Write buffer on the GPU. 
	 */
	template <typename T>
	FUnorderedAccessViewRHIRef InitUAVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	/** 
	 * Update the buffers for the coarse particles, releasing the old ones and creating new ones. 
	 */
	void UpdateCoarseParticleBuffers(
		FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData);

	/** 
	 * Update the buffers for the hashtable, releasing the old ones and creating new ones. 
	 */
	void UpdateHashTableBuffers(
		FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices);

	/** 
	 * The reference to the SRV buffer containing Coarse Particles. 
	 */
	FShaderResourceViewRHIRef CoarseParticleBufferRef;

	/** 
	 * The reference to the SRV buffer containing Active Voxels. 
	 */
	FShaderResourceViewRHIRef ActiveVoxelIndicesBufferRef;

	/** 
	 * The reference to the UAV buffer containing the hashtable data. 
	 */
	FUnorderedAccessViewRHIRef HashTableBufferRef;

	/**
	 * The reference to the UAV buffer containing data on which indices are 
	 * occupied in the hashtable buffer.
	 */
	FUnorderedAccessViewRHIRef HashTableOccupancyBufferRef;

	uint32 NumAllocatedElementsInCoarseParticleBuffer;
	uint32 NumAllocatedElementsInActiveVoxelBuffer;
};

struct FPUArrays
{
	FPUArrays()
	{
	}

	TArray<FCoarseParticle> CoarseParticles;
	TArray<FIntVector4> ActiveVoxelIndices;

	float VoxelSize = 0;
	float FineParticleMass = 0;
	float FineParticleRadius = 0;
	float EaseStepSize = 0;
	float NominalRadius = 0;
	int TableSize = 0;
};

/** 
 * Struct containing all the data which is needed for the Particle
 * Upsampling Data Interface.
 */
struct FParticleUpsamplingData
{
	static const uint32 INITIAL_CP_BUFFER_SIZE = 1024;
	static const uint32 INITIAL_VOXEL_BUFFER_SIZE = 1024;

	void Init(FNiagaraSystemInstance* SystemInstance);
	void Release();

	FPUArrays PUArrays;

	// We use shared pointer here since both the proxy and the NDI will reference this buffer.
	std::shared_ptr<FPUBuffers> PUBuffers;
};
