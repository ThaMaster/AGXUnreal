// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{

}

void FPUBuffers::ReleaseRHI()
{

}

FString FPUBuffers::GetFriendlyName() const
{

}

template <typename T>
FShaderResourceViewRHIRef FPUBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{

}

template <typename T>
FUnorderedAccessViewRHIRef FPUBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{

}

void FPUBuffers::UpdateCoarseParticleBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData,
	uint32 NewElementCount, bool NeedsResize)
{

}

void FPUBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices,
	uint32 NewElementCount, bool NeedsResize)
{

}

FPUArrays::FPUArrays()
{
	CoarseParticles.SetNumZeroed(NumElementsInCoarseParticleBuffer);
	ActiveVoxelIndices.SetNumZeroed(NumElementsInActiveVoxelBuffer);
}

void FPUArrays::CopyFrom(const FPUArrays* Other)
{
	CoarseParticles = Other->CoarseParticles;
	ActiveVoxelIndices = Other->ActiveVoxelIndices;
	
	NumElementsInActiveVoxelBuffer = Other->NumElementsInActiveVoxelBuffer;
	NumElementsInCoarseParticleBuffer = Other->NumElementsInCoarseParticleBuffer;

	Time = Other->Time;
	VoxelSize = Other->VoxelSize;
	FineParticleMass = Other->FineParticleMass;
	FineParticleRadius = Other->FineParticleRadius;
	EaseStepSize = Other->EaseStepSize;
	NominalRadius = Other->NominalRadius;
	TableSize = Other->TableSize;

	NeedsCPResize = Other->NeedsCPResize;
	NeedsVoxelResize = Other->NeedsVoxelResize;
}

void FPUArrays::SetNewTime(int NewTime)
{
	Time = NewTime;
}

void FParticleUpsamplingData::Init(FNiagaraSystemInstance* SystemInstance)
{
	PUBuffers = nullptr;
	if (SystemInstance)
	{
		PUArrays = new FPUArrays();
		PUBuffers = new FPUBuffers();
		BeginInitResource(PUBuffers);
	}
}

void FParticleUpsamplingData::Update(
	FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherData)
{
	if (SystemInstance)
	{
		PUArrays->CopyFrom(OtherData);
		PUArrays->SetNewTime((int) std::time(0));
	}
}

void FParticleUpsamplingData::Release()
{
	if (PUBuffers)
	{
		ENQUEUE_RENDER_COMMAND(DeleteResource)(
			[ParamPointerToRelease = PUBuffers](FRHICommandListImmediate& RHICmdList)
			{
				ParamPointerToRelease->ReleaseResource();
				delete ParamPointerToRelease;
			});
		PUBuffers = nullptr;
	}
}
