// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

/** 
 * Get the size of the data that will be passed to render.
 */
int32 FParticleUpsamplingDIProxy::PerInstanceDataPassedToRenderThreadSize() const
{
	return sizeof(FParticleUpsamplingData);
}

void FParticleUpsamplingDIProxy::ProvidePerInstanceDataForRenderThread(
	void* InDataForRenderThread, void* InDataFromGameThread,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	// Initialize the render thread instance data into the pre-allocated memory
	FParticleUpsamplingData* DataForRenderThread = new (InDataForRenderThread) FParticleUpsamplingData;

	// Copy the game thread data to the render thread data
	const FParticleUpsamplingData* DataFromGameThread = static_cast<FParticleUpsamplingData*>(InDataFromGameThread);
	*DataForRenderThread = *DataFromGameThread;
}

void FParticleUpsamplingDIProxy::ConsumePerInstanceDataFromGameThread(
	void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FParticleUpsamplingData* InstanceDataFromGT = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	FParticleUpsamplingData* InstanceData = &SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData->PUBuffers = InstanceDataFromGT->PUBuffers;
	InstanceData->PUArrays = InstanceDataFromGT->PUArrays;

	if (InstanceData != nullptr && InstanceData->PUBuffers)
	{
		if (InstanceData->PUArrays.CoarseParticles.Num() != 0 &&
			InstanceData->PUArrays.ActiveVoxelIndices.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			InstanceData->PUBuffers->UpdateCoarseParticleBuffers(
				RHICmdList, InstanceData->PUArrays.CoarseParticles);

			InstanceData->PUBuffers->UpdateHashTableBuffers(
				RHICmdList, InstanceData->PUArrays.ActiveVoxelIndices);
		}
	}

	// we call the destructor here to clean up the GT data. Without this we could be leaking
	// memory.
	InstanceDataFromGT->~FParticleUpsamplingData();
}

