// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
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

void FPUBuffers::ReleaseRHI()
{
	CoarseParticleBufferRef.SafeRelease();
	ActiveVoxelIndicesBufferRef.SafeRelease();
	HashTableBufferRef.SafeRelease();
	HashTableOccupancyBufferRef.SafeRelease();
}

template <typename T>
FShaderResourceViewRHIRef FPUBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_ShaderResource, CreateInfo);
	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
}

template <typename T>
FUnorderedAccessViewRHIRef FPUBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_UnorderedAccess, CreateInfo);
	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
}

void FPUBuffers::UpdateCoarseParticleBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData)
{
	uint32 ElementCount = CoarseParticleData.Num();
	if (ElementCount == 0 || !CoarseParticleBufferRef.IsValid() ||
		!CoarseParticleBufferRef->GetBuffer()->IsValid())
	{
		return;
	}

	if (NumAllocatedElementsInCoarseParticleBuffer <= ElementCount)
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
	if (BufferBytes < CoarseParticleBufferRef->GetBuffer()->GetSize())
	{
		void* OutputData = RHICmdList.LockBuffer(
			CoarseParticleBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, CoarseParticleData.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(CoarseParticleBufferRef->GetBuffer());
	}
}

void FPUBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices)
{
	uint32 ElementCount = ActiveVoxelIndices.Num();
	if (ElementCount == 0 || !ActiveVoxelIndicesBufferRef.IsValid() ||
		!ActiveVoxelIndicesBufferRef->GetBuffer()->IsValid())
	{
		return;
	}

	if (NumAllocatedElementsInActiveVoxelBuffer <= ElementCount)
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
	if (BufferBytes < ActiveVoxelIndicesBufferRef->GetBuffer()->GetSize())
	{
		void* OutputData = RHICmdList.LockBuffer(
			ActiveVoxelIndicesBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, ActiveVoxelIndices.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(ActiveVoxelIndicesBufferRef->GetBuffer());
	}
}

void FParticleUpsamplingData::Init(FNiagaraSystemInstance* SystemInstance)
{
	if (!SystemInstance)
	{
		return;
	}

	PUBuffers.reset(new FPUBuffers(INITIAL_CP_BUFFER_SIZE, INITIAL_VOXEL_BUFFER_SIZE));
	BeginInitResource(PUBuffers.get());
}

void FParticleUpsamplingData::Release()
{
	if (PUBuffers)
	{
		ReleaseResourceAndFlush(PUBuffers.get());
	}
}
