// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

// Unreal Engine includes.


/** 
 * Get the size of the data that will be passed to render.
 */
int32 FParticleUpsamplingDIProxy::PerInstanceDataPassedToRenderThreadSize() const
{
	return 0;
}

/** 
 * Get the data that will be passed to render
 */
void FParticleUpsamplingDIProxy::ConsumePerInstanceDataFromGameThread(
	void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{

}

/** 
 * Initialize the Proxy data buffer .
 */
void FParticleUpsamplingDIProxy::InitializePerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
} 

/**
 * Destroy the proxy data if necessary.
 */ 
void FParticleUpsamplingDIProxy::DestroyPerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{

}

