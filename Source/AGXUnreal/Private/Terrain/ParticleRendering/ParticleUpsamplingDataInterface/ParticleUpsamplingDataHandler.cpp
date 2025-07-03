// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDataHandler.h"

void FParticleUpsamplingBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Init SRV Buffers
	CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
		RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), NumAllocatedElementsInCoarseParticleBuffer);
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), NumAllocatedElementsInActiveVoxelBuffer);
	
	// Init UAV Buffers
	HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
		RHICmdList, TEXT("HashTableBuffer"), NumAllocatedElementsInActiveVoxelBuffer * 2);
	HashTableOccupancyBufferRef = InitUAVBuffer<int>(
		RHICmdList, TEXT("HashTableOccupancyBuffer"), NumAllocatedElementsInActiveVoxelBuffer * 2);
}

void FParticleUpsamplingBuffers::ReleaseRHI()
{
	CoarseParticleBufferRef.SafeRelease();
	ActiveVoxelIndicesBufferRef.SafeRelease();
	HashTableBufferRef.SafeRelease();
	HashTableOccupancyBufferRef.SafeRelease();
}

template <typename T>
FShaderResourceViewRHIRef FParticleUpsamplingBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_ShaderResource, CreateInfo);
	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
}

template <typename T>
FUnorderedAccessViewRHIRef FParticleUpsamplingBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_UnorderedAccess, CreateInfo);
	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
}

void FParticleUpsamplingBuffers::UpdateCoarseParticleBuffer(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData)
{
	uint32 ElementCount = CoarseParticleData.Num();
	if (ElementCount == 0 || !CoarseParticleBufferRef.IsValid() ||
		!CoarseParticleBufferRef->GetBuffer()->IsValid())
	{
		return;
	}

	if (NumAllocatedElementsInCoarseParticleBuffer < ElementCount)
	{
		// Release old buffer.
		CoarseParticleBufferRef.SafeRelease();

		// Create new, larger buffer.
		NumAllocatedElementsInCoarseParticleBuffer *= 2;
		CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
			RHICmdList, TEXT("CPPositionsAndRadiusBuffer"),
			NumAllocatedElementsInCoarseParticleBuffer);
	}

	const uint32 BufferBytes = sizeof(FCoarseParticle) * ElementCount;
	void* OutputData = RHICmdList.LockBuffer(
		CoarseParticleBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

	FMemory::Memcpy(OutputData, CoarseParticleData.GetData(), BufferBytes);
	RHICmdList.UnlockBuffer(CoarseParticleBufferRef->GetBuffer());
}

void FParticleUpsamplingBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices)
{
	uint32 ElementCount = ActiveVoxelIndices.Num();
	if (ElementCount == 0 || !ActiveVoxelIndicesBufferRef.IsValid() ||
		!ActiveVoxelIndicesBufferRef->GetBuffer()->IsValid())
	{
		return;
	}

	if (NumAllocatedElementsInActiveVoxelBuffer < ElementCount)
	{
		// Release old buffers.
		ActiveVoxelIndicesBufferRef.SafeRelease();
		HashTableBufferRef.SafeRelease();
		HashTableOccupancyBufferRef.SafeRelease();

		// Create new, larger buffers.
		NumAllocatedElementsInActiveVoxelBuffer *= 2;

		ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
			RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), NumAllocatedElementsInActiveVoxelBuffer);
		HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
			RHICmdList, TEXT("HashtableBuffer"), NumAllocatedElementsInActiveVoxelBuffer * 2);
		HashTableOccupancyBufferRef = InitUAVBuffer<int>(
			RHICmdList, TEXT("HTOccupancy"), NumAllocatedElementsInActiveVoxelBuffer * 2);
	}

	const uint32 BufferBytes = sizeof(FIntVector4) * ElementCount;
	void* OutputData = RHICmdList.LockBuffer(
		ActiveVoxelIndicesBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

	FMemory::Memcpy(OutputData, ActiveVoxelIndices.GetData(), BufferBytes);
	RHICmdList.UnlockBuffer(ActiveVoxelIndicesBufferRef->GetBuffer());
}

void FParticleUpsamplingDataHandler::Init(FNiagaraSystemInstance* SystemInstance)
{
	if (!SystemInstance)
	{
		return;
	}

	Buffers.reset(new FParticleUpsamplingBuffers(
		INITIAL_COARSE_PARTICLE_BUFFER_SIZE, INITIAL_ACTIVE_VOXEL_BUFFER_SIZE));
	BeginInitResource(Buffers.get());
}

void FParticleUpsamplingDataHandler::Release()
{
	if (Buffers)
	{
		ReleaseResourceAndFlush(Buffers.get());
	}
}
