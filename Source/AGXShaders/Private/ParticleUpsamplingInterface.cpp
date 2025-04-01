#include "ParticleUpsamplingInterface.h"
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

#define LOCTEXT_NAMESPACE "NDIParticleUpsampling"

const FName UParticleUpsamplingInterface::UpdateGridName(TEXT("UpdateGrid"));
const FName UParticleUpsamplingInterface::ApplyParticleMassName(TEXT("ApplyParticleMass"));
const FName UParticleUpsamplingInterface::SpawnParticlesName(TEXT("SpawnParticles")); // Maybe change to get room?
const FName UParticleUpsamplingInterface::MoveParticlesName(TEXT("MoveParticles"));
const FName UParticleUpsamplingInterface::ClearTableName(TEXT("ClearTable"));
const FName UParticleUpsamplingInterface::GetVoxelPositionAndRoomName(TEXT("GetVoxelPositionAndRoom"));
const FName UParticleUpsamplingInterface::GetFineParticleRadiusName(TEXT("GetFineParticleRadius"));
const FName UParticleUpsamplingInterface::IsFineParticleAliveName(TEXT("IsFineParticleAlive"));
const FName UParticleUpsamplingInterface::GetCoarseParticleInfoName(TEXT("GetCoarseParticleInfo"));
const FName UParticleUpsamplingInterface::GetNominalRadiusName(TEXT("GetNominalRadius"));
static const
	TCHAR* ParticleUpsamplingTemplateShaderFile =
	TEXT("/AGXShadersShaders/ParticleUpsampling.ush");

// CoarseParticleBuffer -> Fås av CPUn varje tidssteg
// FineParticleBuffer -> Statisk, persistant, stor nog många bytes, byggs up i GPUn. (Se hur de hanteras i Unity/c#).
// ActiveVoxels -> Buffer/Lista fås av CPUn varje tidsteg, AGX (kanske inte finns men skulle vara pog att ha)
// ActiveVoxel Hashmappen -> byggs upp utifrån ActiveVoxel Buffern som skickas från CPUn

// --------------------------------------------------------------- //
// ------------------------- DATA BUFFERS ------------------------ //
// --------------------------------------------------------------- //
template <typename T>
FShaderResourceViewRHIRef FPUBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	TArray<T> DataArray;
	DataArray.SetNumZeroed(ElementCount);

	FResourceArrayUploadArrayView ResourceData(DataArray.GetData(), sizeof(T) * DataArray.Num());
	FRHIResourceCreateInfo CreateInfo(InDebugName, &ResourceData);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(sizeof(T), sizeof(T) * DataArray.Num(), BUF_ShaderResource, CreateInfo);

	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
}

template <typename T>
FUnorderedAccessViewRHIRef FPUBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	TArray<T> DataArray;
	DataArray.SetNumZeroed(ElementCount);

	FResourceArrayUploadArrayView ResourceData(DataArray.GetData(), sizeof(T) * DataArray.Num());
	FRHIResourceCreateInfo CreateInfo(InDebugName, &ResourceData);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(sizeof(T), sizeof(T) * DataArray.Num(), BUF_UnorderedAccess,CreateInfo);

	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
}

void FPUBuffers::UpdateCoarseParticleBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData, uint32 NewElementCount, bool NeedsResize)
{
	uint32 ElementCount = CoarseParticleData.Num();
	if (ElementCount > 0 && CoarseParticleBufferRef.IsValid() && CoarseParticleBufferRef->GetBuffer()->IsValid())
	{
		if (NeedsResize)
		{
			CoarseParticleBufferRef.SafeRelease();
			CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
				RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), NewElementCount);
		}

		const uint32 BufferBytes = sizeof(FCoarseParticle) * ElementCount;
		void* OutputData = RHICmdList.LockBuffer(
			CoarseParticleBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, CoarseParticleData.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(CoarseParticleBufferRef->GetBuffer());
	}
}

void FPUBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices, uint32 NewElementCount, bool NeedsResize)
{
	uint32 ElementCount = ActiveVoxelIndices.Num();
	if (ElementCount > 0 && ActiveVoxelIndicesBufferRef.IsValid() && ActiveVoxelIndicesBufferRef->GetBuffer()->IsValid())
	{
		if (NeedsResize)
		{
			ActiveVoxelIndicesBufferRef.SafeRelease();
			HashTableBufferRef.SafeRelease();
			HTOccupancyBufferRef.SafeRelease();
			ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
				RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), NewElementCount);
			HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
				RHICmdList, TEXT("HashtableBuffer"), NewElementCount);
			HTOccupancyBufferRef =
				InitUAVBuffer<int>(RHICmdList, TEXT("HTOccupancy"), NewElementCount);
		}
		const uint32 BufferBytes = sizeof(FIntVector4) * ElementCount;
		void* OutputData = RHICmdList.LockBuffer(
			ActiveVoxelIndicesBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, ActiveVoxelIndices.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(ActiveVoxelIndicesBufferRef->GetBuffer());
	}
}

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Particle Buffers
	CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
		RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE * 2);

	// HashTable Buffers
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
		RHICmdList, TEXT("HashtableBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTOccupancyBufferRef =
		InitUAVBuffer<int>(RHICmdList, TEXT("HTOccupancy"), INITIAL_VOXEL_BUFFER_SIZE * 2);
}

void FPUBuffers::ReleaseRHI()
{
	// Particle Buffers
	CoarseParticleBufferRef.SafeRelease();

	// HashTable Buffers
	ActiveVoxelIndicesBufferRef.SafeRelease();

	HashTableBufferRef.SafeRelease();
	HTOccupancyBufferRef.SafeRelease();
	
}

// --------------------------------------------------------------- //
// ------------------------- DATA STRUCT ------------------------- //
// --------------------------------------------------------------- //

void FPUData::Init(FNiagaraSystemInstance* SystemInstance)
{
	PUBuffers = nullptr;
	if (SystemInstance)
	{
		PUArrays = new FPUArrays();
		PUBuffers = new FPUBuffers();
		BeginInitResource(PUBuffers);
	}
}

void FPUData::Update(FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherData)
{
	if (SystemInstance)
	{
		PUArrays->CopyFrom(OtherData);
		PUArrays->Time = (int) std::time(0);
	}
}

void FPUData::Release()
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

// --------------------------------------------------------------- //
// ----------------------- INTERFACE PROXY ----------------------- //
// --------------------------------------------------------------- //

void FParticleUpsamplingProxy::InitializePerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());
	check(!SystemInstancesToInstanceData_RT.Contains(SystemInstance));

	SystemInstancesToInstanceData_RT.Add(SystemInstance);
}

void FParticleUpsamplingProxy::DestroyPerInstanceData(const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());

	SystemInstancesToInstanceData_RT.Remove(SystemInstance);
}

/** Update the buffers with the arrays here! */
void FParticleUpsamplingProxy::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	check(SystemInstancesToInstanceData_RT.Contains(Context.GetSystemInstanceID()));

	FPUData* ProxyData = SystemInstancesToInstanceData_RT.Find(Context.GetSystemInstanceID());
	if (ProxyData != nullptr && ProxyData->PUBuffers)
	{
		if (Context.GetSimStageData().bFirstStage &&
			ProxyData->PUArrays->CoarseParticles.Num() != 0 &&
			ProxyData->PUArrays->ActiveVoxelIndices.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			ProxyData->PUBuffers->UpdateCoarseParticleBuffers(
				RHICmdList, 
				ProxyData->PUArrays->CoarseParticles, 
				ProxyData->PUArrays->NumElementsInCoarseParticleBuffer,
				ProxyData->PUArrays->NeedsCPResize);

			ProxyData->PUBuffers->UpdateHashTableBuffers(
				RHICmdList,
				ProxyData->PUArrays->ActiveVoxelIndices,
				ProxyData->PUArrays->NumElementsInActiveVoxelBuffer,
				ProxyData->PUArrays->NeedsVoxelResize);
		}
	}
}

void FParticleUpsamplingProxy::ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FPUData* InstanceDataFromGT = static_cast<FPUData*>(PerInstanceData);
	FPUData& InstanceData = SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData.PUBuffers = InstanceDataFromGT->PUBuffers;
	InstanceData.PUArrays = new FPUArrays();
	InstanceData.PUArrays->CopyFrom(InstanceDataFromGT->PUArrays);

	// we call the destructor here to clean up the GT data. Without this we could be leaking
	// memory.
	InstanceDataFromGT->~FPUData();
}

// --------------------------------------------------------------- //
// ---------------- PARTICLE Upsampling INTERFACE ----------------- //
// --------------------------------------------------------------- //

/** NDI Constructor. */
UParticleUpsamplingInterface::UParticleUpsamplingInterface(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpsamplingProxy());
	LocalData = new FPUArrays();
}

/**
 * This function initializes the PerInstanceData for each instance of this NDI.
 * This means that this function will run when hitting the start button.
 */
bool UParticleUpsamplingInterface::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* PUData = static_cast<FPUData*>(PerInstanceData);
	PUData->Init(SystemInstance);
	LocalData = new FPUArrays();
	return true;
}

/** 
 * This function cleans the data on the RT for each instance of this NDI. 
 * This means that this function will run hitting the pause button.
 */
void UParticleUpsamplingInterface::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* InstanceData = static_cast<FPUData*>(PerInstanceData);

	InstanceData->Release();
	InstanceData->~FPUData();
	FParticleUpsamplingProxy* ThisProxy = GetProxyAs<FParticleUpsamplingProxy>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData)
	(
		[ThisProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			FPUData* ProxyData = ThisProxy->SystemInstancesToInstanceData_RT.Find(InstanceID);

			if (ProxyData != nullptr && ProxyData->PUArrays)
			{
				ThisProxy->SystemInstancesToInstanceData_RT.Remove(InstanceID);
				delete ProxyData->PUArrays;
			}
		});
}
/** This function runs every tick for every instance of this NDI. */
bool UParticleUpsamplingInterface::PerInstanceTick(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FPUData* PUData = static_cast<FPUData*>(PerInstanceData);

	if (!PUData)
	{
		return true;
	}

	PUData->Update(SystemInstance, LocalData);
	LocalData->NeedsVoxelResize = false;
	LocalData->NeedsCPResize = false;
	return false;
}

/** This function runs every tick, post simulate, for every instance of this NDI. */
bool UParticleUpsamplingInterface::PerInstanceTickPostSimulate(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FPUData* PUData = static_cast<FPUData*>(PerInstanceData);

	if (!PUData)
	{
		return true;
	}
	return false;
}

void UParticleUpsamplingInterface::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	FPUData* RenderThreadData = static_cast<FPUData*>(DataForRenderThread);
	FPUData* GameThreadData = static_cast<FPUData*>(PerInstanceData);

	if (RenderThreadData && GameThreadData)
	{
		RenderThreadData->PUBuffers = GameThreadData->PUBuffers;
		RenderThreadData->PUArrays = new FPUArrays();
		RenderThreadData->PUArrays->CopyFrom(GameThreadData->PUArrays);
	}
}

/** Funcition for registering our custom DI with Niagara */
void UParticleUpsamplingInterface::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags =
			ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

#if WITH_EDITORONLY_DATA
/** This lists all the functions that will be visible when using the NDI. */
void UParticleUpsamplingInterface::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = UpdateGridName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));

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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Ease")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
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
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(FNiagaraVariable(
			FNiagaraTypeDefinition::GetFloatDef(), TEXT("NominalRadius")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}
}
#endif

#if WITH_EDITORONLY_DATA

// this lets the niagara compiler know that it needs to recompile an effect when our hlsl file
// changes
bool UParticleUpsamplingInterface::AppendCompileHash(
	FNiagaraCompileHashVisitor* InVisitor) const
{
	bool bSuccess = Super::AppendCompileHash(InVisitor);
	bSuccess &= InVisitor->UpdateShaderFile(ParticleUpsamplingTemplateShaderFile);
	bSuccess &= InVisitor->UpdateShaderParameters<FShaderParameters>();
	return bSuccess;
}

bool UParticleUpsamplingInterface::GetFunctionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
	FString& OutHLSL)
{
	return	FunctionInfo.DefinitionName == UpdateGridName ||
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

/** Loads our hlsl template script file and replaces all template arguments accordingly. */
void UParticleUpsamplingInterface::GetParameterDefinitionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	const TMap<FString, FStringFormatArg> TemplateArgs = {
		{TEXT("ParameterName"), ParamInfo.DataInterfaceHLSLSymbol},
	};
	AppendTemplateHLSL(OutHLSL, ParticleUpsamplingTemplateShaderFile, TemplateArgs);
}
#endif

// This fills in the expected parameter bindings we use to send data to the GPU
void UParticleUpsamplingInterface::BuildShaderParameters(
	FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

// This fills in the parameters to send to the GPU
void UParticleUpsamplingInterface::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FParticleUpsamplingProxy& DataInterfaceProxy = Context.GetProxy<FParticleUpsamplingProxy>();
	FPUData& PUData = DataInterfaceProxy.SystemInstancesToInstanceData_RT.FindChecked(Context.GetSystemInstanceID());
	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();

	// Particle Shader Parameters
	ShaderParameters->CoarseParticles		=	PUData.PUBuffers->CoarseParticleBufferRef;
	ShaderParameters->NumCoarseParticles	=	PUData.PUArrays->CoarseParticles.Num();
	ShaderParameters->FineParticleMass		=	PUData.PUArrays->FineParticleMass;
	ShaderParameters->FineParticleRadius	=	PUData.PUArrays->FineParticleRadius;
	ShaderParameters->NominalRadius			=	PUData.PUArrays->NominalRadius;

	// HashTable Shader Parameters
	ShaderParameters->ActiveVoxelIndices	=	PUData.PUBuffers->ActiveVoxelIndicesBufferRef;
	ShaderParameters->NumActiveVoxels		=	PUData.PUArrays->ActiveVoxelIndices.Num();

	ShaderParameters->HashTable				=	PUData.PUBuffers->HashTableBufferRef;
	ShaderParameters->HTOccupancy			=	PUData.PUBuffers->HTOccupancyBufferRef;

	ShaderParameters->TableSize				=	PUData.PUArrays->NumElementsInActiveVoxelBuffer;
	ShaderParameters->VoxelSize				=	PUData.PUArrays->VoxelSize;

	// Other Variables
	ShaderParameters->Time					=	PUData.PUArrays->Time;
	ShaderParameters->AnimationSpeed		=	PUData.PUArrays->EaseStepSize;
}

// ---------- STATIC FUNCTIONALITY BELOW ----------

const float UParticleUpsamplingInterface::PACKING_RATIO = 0.67f;
FPUArrays* UParticleUpsamplingInterface::LocalData = new FPUArrays();

void UParticleUpsamplingInterface::SetCoarseParticles(TArray<FCoarseParticle> NewCoarseParticles)
{
	uint32 ArraySize = NewCoarseParticles.Num();
	if (LocalData->NumElementsInCoarseParticleBuffer < ArraySize)
	{
		LocalData->NumElementsInCoarseParticleBuffer *= 2;
		LocalData->NeedsCPResize = true;
	}
	LocalData->CoarseParticles.SetNumZeroed(NewCoarseParticles.Num());
	LocalData->CoarseParticles = NewCoarseParticles;
}

void UParticleUpsamplingInterface::SetActiveVoxelIndices(TArray<FIntVector4> AVIs)
{
	uint32 ArraySize = AVIs.Num();
	if (LocalData->NumElementsInActiveVoxelBuffer < ArraySize)
	{
		LocalData->NumElementsInActiveVoxelBuffer *= 2;
		LocalData->NeedsVoxelResize = true;
	}
	LocalData->ActiveVoxelIndices.SetNumZeroed(AVIs.Num());
	LocalData->ActiveVoxelIndices = AVIs;
}

int UParticleUpsamplingInterface::GetHashTableSize()
{
	return LocalData->NumElementsInActiveVoxelBuffer;
}

void UParticleUpsamplingInterface::RecalculateFineParticleProperties(
	float Upsampling, float ElementSize, float ParticleDensity)
{
	LocalData->NominalRadius = FMath::Pow(3.0f * PACKING_RATIO / (4.0f * PI), 1.0f / 3.0f) * ElementSize;
	LocalData->FineParticleRadius = LocalData->NominalRadius / FMath::Pow(Upsampling, 1.0f / 3.0f);
	float NominalMass = ParticleDensity * 4.0f / 3.0f * PI * FMath::Pow(LocalData->NominalRadius, 3.0f);
	LocalData->FineParticleMass = NominalMass / Upsampling;

	UE_LOG(
		LogTemp, Warning,
		TEXT("NominalRadius: %f, FineParticleRadius: %f, NominalMass: %f, FineParticleMass: %f, "
			 "Upsampling: %f, ElementSize: %f, ParticleDensity: %f"),
		LocalData->NominalRadius, LocalData->FineParticleRadius, NominalMass,
		LocalData->FineParticleMass, Upsampling, ElementSize, ParticleDensity);
}

void UParticleUpsamplingInterface::SetStaticVariables(float VoxelSize, float EaseStepSize)
{
	LocalData->VoxelSize = VoxelSize;
	LocalData->EaseStepSize = EaseStepSize;
}


#undef LOCTEXT_NAMESPACE
