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
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = 
			ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

bool UAGX_ParticleUpsamplingDI::CanExecuteOnTarget(ENiagaraSimTarget Target) const
{
	return true;
}

/**
 * This fills in the expected parameter bindings we use to send data to the GPU.
 */
void UAGX_ParticleUpsamplingDI::BuildShaderParameters(
	FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

/**
 * This fills in the parameters with data to send to the GPU.
 */
void UAGX_ParticleUpsamplingDI::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FParticleUpsamplingDIProxy& DataInterfaceProxy = Context.GetProxy<FParticleUpsamplingDIProxy>();
	FParticleUpsamplingData& Data = DataInterfaceProxy.SystemInstancesToInstanceData_RT.FindChecked(
		Context.GetSystemInstanceID());
	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();

	// Particle Shader Parameters
	ShaderParameters->CoarseParticles		= Data.PUBuffers->CoarseParticleBufferRef;
	ShaderParameters->NumCoarseParticles	= Data.PUArrays->CoarseParticles.Num();
	ShaderParameters->FineParticleMass		= Data.PUArrays->FineParticleMass;
	ShaderParameters->FineParticleRadius	= Data.PUArrays->FineParticleRadius;
	ShaderParameters->NominalRadius			= Data.PUArrays->NominalRadius;

	// HashTable Shader Parameters
	ShaderParameters->ActiveVoxelIndices	= Data.PUBuffers->ActiveVoxelIndicesBufferRef;
	ShaderParameters->NumActiveVoxels		= Data.PUArrays->ActiveVoxelIndices.Num();

	ShaderParameters->HashTable				= Data.PUBuffers->HashTableBufferRef;
	ShaderParameters->HTOccupancy			=Data.PUBuffers->HashTableOccupancyBufferRef;

	ShaderParameters->TableSize				= Data.PUArrays->NumElementsInActiveVoxelBuffer;
	ShaderParameters->VoxelSize				= Data.PUArrays->VoxelSize;

	// Other Variables
	ShaderParameters->Time					= Data.PUArrays->Time;
	ShaderParameters->AnimationSpeed		= Data.PUArrays->EaseStepSize;
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
	FParticleUpsamplingData* InstanceData = static_cast<FParticleUpsamplingData*>(PerInstanceData);

	InstanceData->Release();
	InstanceData->~FParticleUpsamplingData();
	FParticleUpsamplingDIProxy* ThisProxy = GetProxyAs<FParticleUpsamplingDIProxy>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData)
	(
		[ThisProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			FParticleUpsamplingData* ProxyData =
				ThisProxy->SystemInstancesToInstanceData_RT.Find(InstanceID);
			if (ProxyData != nullptr && ProxyData->PUArrays)
			{
				ThisProxy->SystemInstancesToInstanceData_RT.Remove(InstanceID);
				delete ProxyData->PUArrays;
			}
		});
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
	check(SystemInstance);
	FParticleUpsamplingData* Data = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	if (!Data)
	{
		return true;
	}

	Data->Update(SystemInstance, LocalData);
	LocalData->NeedsCPResize = false;
	LocalData->NeedsVoxelResize = false;
	return false;
}

/** 
 * This function runs every tick, post simulate, for every instance of this NDI. 
 */
bool UAGX_ParticleUpsamplingDI::PerInstanceTickPostSimulate(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FParticleUpsamplingData* Data = static_cast<FParticleUpsamplingData*>(PerInstanceData);

	if (!Data)
	{
		return true;
	}

	return false;
}

void UAGX_ParticleUpsamplingDI::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	FParticleUpsamplingData* RenderThreadData =
		static_cast<FParticleUpsamplingData*>(DataForRenderThread);
	FParticleUpsamplingData* GameThreadData =
		static_cast<FParticleUpsamplingData*>(PerInstanceData);

	if (RenderThreadData && GameThreadData)
	{
		RenderThreadData->PUBuffers = GameThreadData->PUBuffers;
		RenderThreadData->PUArrays = new FPUArrays();
		RenderThreadData->PUArrays->CopyFrom(GameThreadData->PUArrays);
	}
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
	return FunctionInfo.DefinitionName == UpdateGridName ||
		   FunctionInfo.DefinitionName == ApplyParticleMassName ||
		   FunctionInfo.DefinitionName == SpawnParticlesName ||
		   FunctionInfo.DefinitionName == MoveParticlesName ||
		   FunctionInfo.DefinitionName == GetVoxelPositionAndRoomName ||
		   FunctionInfo.DefinitionName == ClearTableName ||
		   FunctionInfo.DefinitionName == GetFineParticleRadiusName ||
		   FunctionInfo.DefinitionName == IsFineParticleAliveName ||
		   FunctionInfo.DefinitionName == GetNominalRadiusName ||
		   FunctionInfo.DefinitionName == GetCoarseParticleInfoName;
}

/** 
 * This lists all the functions that will be visible when using the NDI. 
 */
void UAGX_ParticleUpsamplingDI::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = UpdateGridName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ApplyParticleMassName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePosition")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEase")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEaseNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = SpawnParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ShouldSpawn")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ParticleRoom")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("VoxelIndex")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = MoveParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ParticleIndex")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePosition")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEase")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("DeltaTime")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePositionNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEaseNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticleVelocityNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ClearTableName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetVoxelPositionAndRoomName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("VoxelPosition")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetFineParticleRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FineParticleRadius")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = IsFineParticleAliveName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Ease")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetCoarseParticleInfoName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("CoarseParticleIndex")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec4Def(), TEXT("ParticlePositionAndRadius")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticleVelocity")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetNominalRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NominalRadius")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}
}
#endif