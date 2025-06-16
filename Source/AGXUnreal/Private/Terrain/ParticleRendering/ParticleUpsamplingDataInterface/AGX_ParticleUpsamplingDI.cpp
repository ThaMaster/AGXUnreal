// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/AGX_ParticleUpsamplingDI.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// Unreal Engine includes.
#include "NiagaraCompileHashVisitor.h"
#include "NiagaraTypes.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraParameterStore.h"
#include "NiagaraSimStageData.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "RHIUtilities.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "NiagaraDataInterfaceArray.h"

UAGX_ParticleUpsamplingDI::UAGX_ParticleUpsamplingDI(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpsamplingDIProxy());
	LocalData = new FPUArrays();
}

/**
 * Funcition for registering our custom DI with Niagara
 */
void UAGX_ParticleUpsamplingDI::PostInitProperties()
{
}

bool UAGX_ParticleUpsamplingDI::CanExecuteOnTarget(ENiagaraSimTarget Target) const
{
	return false;
}

/**
 * This fills in the expected parameter bindings we use to send data to the GPU.
 */
void UAGX_ParticleUpsamplingDI::BuildShaderParameters(
	FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
}

/**
 * This fills in the parameters with data to send to the GPU.
 */
void UAGX_ParticleUpsamplingDI::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
}

/**
 * This function initializes the PerInstanceData for each instance of this NDI.
 * This means that this function will run when hitting the start button.
 */
bool UAGX_ParticleUpsamplingDI::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FParticleUpsamplingData* Data = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	Data->Init(SystemInstance);
	LocalData = new FPUArrays();
	return true;
}

/**
 * This function cleans the data on the RT for each instance of this NDI.
 * This means that this function will run hitting the pause button.
 */
void UAGX_ParticleUpsamplingDI::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
}

int32 UAGX_ParticleUpsamplingDI::PerInstanceDataSize() const
{
	return sizeof(FParticleUpsamplingData);
}

bool UAGX_ParticleUpsamplingDI::HasPreSimulateTick() const
{
	return true;

}

bool UAGX_ParticleUpsamplingDI::HasPostSimulateTick() const
{
	return true;
}

/** 
 * This function runs every tick for every instance of this NDI. 
 */
bool UAGX_ParticleUpsamplingDI::PerInstanceTick(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	return false;
}

/** 
 * This function runs every tick, post simulate, for every instance of this NDI. 
 */
bool UAGX_ParticleUpsamplingDI::PerInstanceTickPostSimulate(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	return false;
}

void UAGX_ParticleUpsamplingDI::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{

}

#if WITH_EDITORONLY_DATA

/** 
 * This lets the Niagara compiler know that it needs to recompile an effect when
 * our HLSL file changes.
 */
bool UAGX_ParticleUpsamplingDI::AppendCompileHash(
	FNiagaraCompileHashVisitor* InVisitor) const
{
	return false;
}

/** 
 * Loads our HLSL template script file and replaces all template arguments accordingly.
 */
void UAGX_ParticleUpsamplingDI::GetParameterDefinitionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{

}

/**
 * 
 */ 
bool UAGX_ParticleUpsamplingDI::GetFunctionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
	FString& OutHLSL)
{
	return false;
}

/** 
 * This lists all the functions that will be visible when using the NDI. 
 */
void UAGX_ParticleUpsamplingDI::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{

}
#endif