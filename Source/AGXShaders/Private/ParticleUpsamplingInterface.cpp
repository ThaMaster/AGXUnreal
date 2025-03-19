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

const FName UParticleUpsamplingInterface::GetFineParticlePositionAndRadiusName(TEXT("GetFineParticlePositionAndRadius"));
const FName UParticleUpsamplingInterface::GetFineParticleVelocityAndMassName(TEXT("GetFineParticleVelocityAndMass"));
const FName UParticleUpsamplingInterface::GetNumCoarseParticlesName(TEXT("GetNumCoarseParticles"));
const FName UParticleUpsamplingInterface::GetActiveVoxelIndexName(TEXT("GetActiveVoxelIndex"));
const FName UParticleUpsamplingInterface::GetFineParticleRadiusName(TEXT("GetFineParticleRadius"));
const FName UParticleUpsamplingInterface::UpdateGridName(TEXT("UpdateGrid"));
const FName UParticleUpsamplingInterface::LookupRoomName(TEXT("LookupRoom"));

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

template <typename T>
void FPUBuffers::UpdateSRVBuffer(FRHICommandListBase& RHICmdList, const TArray<T>& InputData, FShaderResourceViewRHIRef& OutputBuffer)
{
	uint32 ElementCount = InputData.Num();
	if (ElementCount > 0 && OutputBuffer->GetBuffer()->IsValid())
	{
		FRHIBuffer* SRVBuffer = OutputBuffer->GetBuffer();
		const uint32 BufferBytes = sizeof(T) * ElementCount;

		if (SRVBuffer->GetSize() < BufferBytes)
		{
			UE_LOG(
				LogTemp, Warning,
				TEXT("[Warning] Trying to store %d bytes of type %hs but buffer is of size %d bytes."),
				BufferBytes, typeid(T).name(), SRVBuffer->GetSize());
			return;
		}
		void* OutputData =
			RHICmdList.LockBuffer(OutputBuffer->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, InputData.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(OutputBuffer->GetBuffer());
	}
}

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Particle Buffers
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), INITIAL_VOXEL_BUFFER_SIZE);
	CPPositionsAndRadiusBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE);
	CPVelocitiesAndMassesBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("CPVelocitiesAndMassesBuffer"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE);

	// HashTable Buffers
	HashTableSize = INITIAL_VOXEL_BUFFER_SIZE * 2;
	HTIndexAndRoomBufferRef = InitUAVBuffer<FIntVector4>(RHICmdList, TEXT("HTIndexAndRoomBuffer"), HashTableSize);
	HTPositionAndMassBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTPositionAndMassBuffer"), HashTableSize);
	HTVelocityBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTVelocityBuffer"), HashTableSize);
	HTMinBoundBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTMinBoundBuffer"), HashTableSize);
	HTMaxBoundBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTMaxBoundBuffer"), HashTableSize);
	HTInnerMinBoundBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTInnerMinBoundBuffer"), HashTableSize);
	HTInnerMaxBoundBufferRef = InitUAVBuffer<FVector4f>(RHICmdList, TEXT("HTInnerMaxBoundBuffer"), HashTableSize);
	HTOccupancyBufferRef = InitUAVBuffer<uint32>(RHICmdList, TEXT("HTOccupancy"), HashTableSize);
}

void FPUBuffers::ReleaseRHI()
{
	// Particle Buffers
	ActiveVoxelIndicesBufferRef.SafeRelease();
	CPPositionsAndRadiusBufferRef.SafeRelease();
	CPVelocitiesAndMassesBufferRef.SafeRelease();

	// HashTable Buffers
	HTIndexAndRoomBufferRef.SafeRelease();
	HTPositionAndMassBufferRef.SafeRelease();
	HTVelocityBufferRef.SafeRelease();
	HTMinBoundBufferRef.SafeRelease();
	HTMaxBoundBufferRef.SafeRelease();
	HTInnerMinBoundBufferRef.SafeRelease();
	HTInnerMaxBoundBufferRef.SafeRelease();
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
		PUArrays->TimeStep = SystemInstance->GetLastRenderTime();
		PUArrays->Time = (int) std::time(0);

		/** Enqueue a render command that resizes the buffers if it is needed. */
		if (OtherData->bActiveVoxelIndicesBufferNeedsResize)
		{
			ENQUEUE_RENDER_COMMAND(FUpdateActiveVoxelIndicesBufferSize)
			(
				[Buffers = PUBuffers, Arrays = PUArrays](FRHICommandListImmediate& RHICmdList) {
					Buffers->ActiveVoxelIndicesBufferRef.SafeRelease();
					Buffers->HTIndexAndRoomBufferRef.SafeRelease();
					Buffers->HTPositionAndMassBufferRef.SafeRelease();
					Buffers->HTVelocityBufferRef.SafeRelease();
					Buffers->HTMinBoundBufferRef.SafeRelease();
					Buffers->HTMaxBoundBufferRef.SafeRelease();
					Buffers->HTInnerMinBoundBufferRef.SafeRelease();
					Buffers->HTInnerMaxBoundBufferRef.SafeRelease();

					Buffers->ActiveVoxelIndicesBufferRef = Buffers->InitSRVBuffer<FVector4f>(
						RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), Arrays->ActiveVoxelIndices.Num());

					Buffers->HashTableSize = Buffers->HashTableSize;

					Buffers->HTIndexAndRoomBufferRef = Buffers->InitUAVBuffer<FIntVector4>(
						RHICmdList, TEXT("HTIndexAndRoomBuffer"), Buffers->HashTableSize);

					Buffers->HTPositionAndMassBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTPositionAndMassBuffer"), Buffers->HashTableSize);

					Buffers->HTVelocityBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTVelocityBuffer"), Buffers->HashTableSize);

					Buffers->HTMinBoundBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTMinBoundBuffer"), Buffers->HashTableSize);

					Buffers->HTMaxBoundBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTMaxBoundBuffer"), Buffers->HashTableSize);

					Buffers->HTInnerMinBoundBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTInnerMinBoundBuffer"), Buffers->HashTableSize);

					Buffers->HTInnerMaxBoundBufferRef = Buffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTInnerMaxBoundBuffer"), Buffers->HashTableSize);
				});
			UE_LOG(LogTemp, Warning, TEXT("HashTable Buffers needed resize!"));
		}

		if (OtherData->bCoarseParticlesBufferNeedsReisze)
		{
			ENQUEUE_RENDER_COMMAND(FUpdateCoarseParticlesBufferSize)
			(
				[Buffers = PUBuffers, Arrays = PUArrays](FRHICommandListImmediate& RHICmdList) {
					Buffers->CPPositionsAndRadiusBufferRef.SafeRelease();
					Buffers->CPPositionsAndRadiusBufferRef = Buffers->InitSRVBuffer<FVector4f>(
						RHICmdList, TEXT("CPPositionsAndRadiusBuffer"),
						Arrays->CPPositionsAndRadius.Num());

					Buffers->CPVelocitiesAndMassesBufferRef.SafeRelease();
					Buffers->CPVelocitiesAndMassesBufferRef = Buffers->InitSRVBuffer<FVector4f>(
						RHICmdList, TEXT("CPVelocitiesAndMassesBuffer"),
						Arrays->CPVelocitiesAndMasses.Num());
				}
			);
			UE_LOG(LogTemp, Warning, TEXT("Coarse Particle Buffers needed resize!"));

		}
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
			ProxyData->PUArrays->CPPositionsAndRadius.Num() != 0 &&
			ProxyData->PUArrays->CPVelocitiesAndMasses.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, ProxyData->PUArrays->ActiveVoxelIndices, 
				ProxyData->PUBuffers->ActiveVoxelIndicesBufferRef);

			ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, ProxyData->PUArrays->CPPositionsAndRadius,
				ProxyData->PUBuffers->CPPositionsAndRadiusBufferRef);

			ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, ProxyData->PUArrays->CPVelocitiesAndMasses,
				ProxyData->PUBuffers->CPVelocitiesAndMassesBufferRef);
		}
	}
}

void FParticleUpsamplingProxy::ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FPUData* InstanceDataFromGT = static_cast<FPUData*>(PerInstanceData);
	FPUData& InstanceData = SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData.PUArrays = InstanceDataFromGT->PUArrays;
	InstanceData.PUBuffers = InstanceDataFromGT->PUBuffers;

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
	
	// Must reset these here!
	LocalData->bActiveVoxelIndicesBufferNeedsResize = false;
	LocalData->bCoarseParticlesBufferNeedsReisze = false;
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
		Sig.Name = GetFineParticlePositionAndRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec4Def(), TEXT("FineParticlePositionAndRadius")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetFineParticleVelocityAndMassName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetVec4Def(), TEXT("FineParticleVelocityAndMass")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetNumCoarseParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumCoarseParticles")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetActiveVoxelIndexName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ActiveVoxelIndex")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetFineParticleRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FineParticleRadius")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = UpdateGridName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = LookupRoomName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

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
	return	FunctionInfo.DefinitionName == GetFineParticlePositionAndRadiusName ||
			FunctionInfo.DefinitionName == GetFineParticleVelocityAndMassName ||
			FunctionInfo.DefinitionName == GetNumCoarseParticlesName ||
			FunctionInfo.DefinitionName == GetActiveVoxelIndexName ||
			FunctionInfo.DefinitionName == GetFineParticleRadiusName ||
			FunctionInfo.DefinitionName == UpdateGridName ||
			FunctionInfo.DefinitionName == LookupRoomName;
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
	ShaderParameters->ActiveVoxelIndices	=	PUData.PUBuffers->ActiveVoxelIndicesBufferRef;
	ShaderParameters->CPPositionsAndRadius	=	PUData.PUBuffers->CPPositionsAndRadiusBufferRef;
	ShaderParameters->CPVelocitiesAndMasses =	PUData.PUBuffers->CPVelocitiesAndMassesBufferRef;

	ShaderParameters->NumActiveVoxels		=	PUData.PUArrays->ActiveVoxelIndices.Num();
	ShaderParameters->NumCoarseParticles	=	PUData.PUArrays->CPVelocitiesAndMasses.Num();
	ShaderParameters->VoxelSize				=	PUData.PUArrays->VoxelSize;
	ShaderParameters->FineParticleMass		=	PUData.PUArrays->FineParticleMass;
	ShaderParameters->FineParticleRadius	=	PUData.PUArrays->FineParticleRadius;
	ShaderParameters->NominalRadius			=	PUData.PUArrays->NominalRadius;

	//ShaderParameters->Time =					PUData.PUArrays->Time;
	//ShaderParameters->TimeStep =				PUData.PUArrays->TimeStep;
	//ShaderParameters->AnimationSpeed		=	PUData.PUArrays->EaseStepSize;

	// HashTable Shader Parameters
	ShaderParameters->HTIndexAndRoom		=	PUData.PUBuffers->HTIndexAndRoomBufferRef;
	ShaderParameters->HTPositionAndMass		=	PUData.PUBuffers->HTPositionAndMassBufferRef;
	ShaderParameters->HTVelocity			=	PUData.PUBuffers->HTVelocityBufferRef;
	ShaderParameters->HTMinBound			=	PUData.PUBuffers->HTMinBoundBufferRef;
	ShaderParameters->HTMaxBound			=	PUData.PUBuffers->HTMaxBoundBufferRef;
	ShaderParameters->HTInnerMinBound		=	PUData.PUBuffers->HTInnerMinBoundBufferRef;
	ShaderParameters->HTInnerMaxBound		=	PUData.PUBuffers->HTInnerMaxBoundBufferRef;
	ShaderParameters->HTOccupancy			=	PUData.PUBuffers->HTOccupancyBufferRef;
	ShaderParameters->TableSize				=	PUData.PUBuffers->HashTableSize;
}

// ---------- STATIC FUNCTIONALITY BELOW ----------

const float UParticleUpsamplingInterface::PACKING_RATIO = 0.67f;
FPUArrays* UParticleUpsamplingInterface::LocalData = new FPUArrays();

void UParticleUpsamplingInterface::SetCoarseParticles(TArray<FVector4f> PositionsAndRadius, TArray<FVector4f> VelocitiesAndMasses)
{
	uint32 ElementSize = PositionsAndRadius.Num();
	uint32 CurrentElementSize = LocalData->CPPositionsAndRadius.Num();
	LocalData->bCoarseParticlesBufferNeedsReisze = CurrentElementSize < ElementSize;

	LocalData->CPPositionsAndRadius.SetNumZeroed(ElementSize);
	LocalData->CPPositionsAndRadius = PositionsAndRadius;

	LocalData->CPVelocitiesAndMasses.SetNumZeroed(ElementSize);
	LocalData->CPVelocitiesAndMasses = VelocitiesAndMasses;
}

void UParticleUpsamplingInterface::SetActiveVoxelIndices(TArray<FVector4f> AVIs)
{
	uint32 ElementSize = AVIs.Num();
	uint32 CurrentElementSize = LocalData->ActiveVoxelIndices.Num();
	LocalData->bActiveVoxelIndicesBufferNeedsResize = CurrentElementSize < ElementSize;
	LocalData->ActiveVoxelIndices.SetNumZeroed(ElementSize);
	LocalData->ActiveVoxelIndices = AVIs;
}

void UParticleUpsamplingInterface::RecalculateFineParticleProperties(
	float Upsampling, float ElementSize, float ParticleDensity)
{
	LocalData->NominalRadius = FMath::Pow(3.0f * PACKING_RATIO / (4.0f * PI), 1.0f / 3.0f) * ElementSize;
	LocalData->FineParticleRadius = LocalData->NominalRadius / FMath::Pow(Upsampling, 1.0f / 3.0f);
	float NominalMass = ParticleDensity * 4.0f / 3.0f * PI * FMath::Pow(LocalData->NominalRadius, 3.0f);
	LocalData->FineParticleMass = NominalMass / Upsampling;
}

void UParticleUpsamplingInterface::SetStaticVariables(float VoxelSize, float EaseStepSize)
{
	LocalData->VoxelSize = VoxelSize;
	LocalData->EaseStepSize = EaseStepSize;
}


#undef LOCTEXT_NAMESPACE
