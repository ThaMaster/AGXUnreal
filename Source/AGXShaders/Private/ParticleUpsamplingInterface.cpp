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

static const
	TCHAR* ParticleUpsamplingTemplateShaderFile =
	TEXT("/AGXShadersShaders/ParticleUpsampling.ush");
static const TCHAR* ConstantsAndDefinesTemplateShaderFile =
	TEXT("/AGXShadersShaders/ConstantsAndDefines.ush");

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
uint32 FPUBuffers::UpdateSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, const TArray<T>& InputData,
	FShaderResourceViewRHIRef& OutputBuffer)
{
	uint32 newElementCount = 0;
	uint32 ElementCount = InputData.Num();
	bool changed = false;
	if (ElementCount > 0 && OutputBuffer->GetBuffer()->IsValid())
	{
		const uint32 BufferBytes = sizeof(T) * ElementCount;
		uint32 BufferSize = OutputBuffer->GetBuffer()->GetSize();
		newElementCount = (BufferSize / sizeof(T));
		while (BufferSize < BufferBytes)
		{
			OutputBuffer.SafeRelease();
			newElementCount = (BufferSize / sizeof(T)) * 2;
			OutputBuffer = InitSRVBuffer<T>(RHICmdList, InDebugName, newElementCount);
			BufferSize = OutputBuffer->GetBuffer()->GetSize();
			changed = true;
		}
		if (changed)
		{
			UE_LOG(LogTemp, Warning, TEXT("SRV Buffer Resized!"));
		}
		void* OutputData =
			RHICmdList.LockBuffer(OutputBuffer->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, InputData.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(OutputBuffer->GetBuffer());
	}
	return newElementCount;
}

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Particle Buffers
	CPPositionsAndRadiusBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE * 2);
	CPVelocitiesAndMassesBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("CPVelocitiesAndMassesBuffer"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE * 2);

	// HashTable Buffers
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FVector4f>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTIndexAndRoomBufferRef = InitUAVBuffer<FVector4f>(
		RHICmdList, TEXT("HTIndexAndRoomBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTVelocityAndMassBufferRef = InitUAVBuffer<FVector4f>(
		RHICmdList, TEXT("HTVelocityAndMassBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTMaxBoundBufferRef = InitUAVBuffer<FVector4f>(
		RHICmdList, TEXT("HTMaxBoundBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTMinBoundBufferRef = InitUAVBuffer<FVector4f>(
		RHICmdList, TEXT("HTMinBoundBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HTOccupancyBufferRef =
		InitUAVBuffer<int>(RHICmdList, TEXT("HTOccupancy"), INITIAL_VOXEL_BUFFER_SIZE * 2);
}

void FPUBuffers::ReleaseRHI()
{
	// Particle Buffers
	CPPositionsAndRadiusBufferRef.SafeRelease();
	CPVelocitiesAndMassesBufferRef.SafeRelease();

	// HashTable Buffers
	ActiveVoxelIndicesBufferRef.SafeRelease();
	HTIndexAndRoomBufferRef.SafeRelease();
	HTVelocityAndMassBufferRef.SafeRelease();
	HTMaxBoundBufferRef.SafeRelease();
	HTMinBoundBufferRef.SafeRelease();
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
		PUArrays->TimeStep = SystemInstance->GetLastRenderTime();
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
			ProxyData->PUArrays->CPPositionsAndRadius.Num() != 0 &&
			ProxyData->PUArrays->CPVelocitiesAndMasses.Num() != 0 &&
			ProxyData->PUArrays->ActiveVoxelIndices.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, 
				TEXT("CPPositionsAndRadiusBuffer"),
				ProxyData->PUArrays->CPPositionsAndRadius,
				ProxyData->PUBuffers->CPPositionsAndRadiusBufferRef);

			ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, 
				TEXT("CPVelocitiesAndMassesBuffer"),
				ProxyData->PUArrays->CPVelocitiesAndMasses,
				ProxyData->PUBuffers->CPVelocitiesAndMassesBufferRef);

			uint32 NewVoxelElementCount = ProxyData->PUBuffers->UpdateSRVBuffer<FVector4f>(
				RHICmdList, 
				TEXT("ActiveVoxelIndicesBuffer"),
				ProxyData->PUArrays->ActiveVoxelIndices,
				ProxyData->PUBuffers->ActiveVoxelIndicesBufferRef);
			UE_LOG(LogTemp, Warning, TEXT("Current ElementSize in Voxel Buffers: %d"), NewVoxelElementCount);

			if (ProxyData->PUArrays->NumElementsInActiveVoxelBuffer < NewVoxelElementCount)
			{
				ProxyData->PUBuffers->HTIndexAndRoomBufferRef.SafeRelease();
				ProxyData->PUBuffers->HTVelocityAndMassBufferRef.SafeRelease();
				ProxyData->PUBuffers->HTMaxBoundBufferRef.SafeRelease();
				ProxyData->PUBuffers->HTMinBoundBufferRef.SafeRelease();
				ProxyData->PUBuffers->HTOccupancyBufferRef.SafeRelease();

				ProxyData->PUBuffers->HTIndexAndRoomBufferRef =
					ProxyData->PUBuffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTIndexAndRoomBuffer"), NewVoxelElementCount);
				ProxyData->PUBuffers->HTVelocityAndMassBufferRef = 
					ProxyData->PUBuffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTVelocityAndMassBuffer"), NewVoxelElementCount);
				ProxyData->PUBuffers->HTMaxBoundBufferRef = 
					ProxyData->PUBuffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTMaxBoundBuffer"), NewVoxelElementCount);
				ProxyData->PUBuffers->HTMinBoundBufferRef = 
					ProxyData->PUBuffers->InitUAVBuffer<FVector4f>(
						RHICmdList, TEXT("HTMinBoundBuffer"), NewVoxelElementCount);
				ProxyData->PUBuffers->HTOccupancyBufferRef =
					ProxyData->PUBuffers->InitUAVBuffer<int>(
						RHICmdList, TEXT("HTOccupancy"), NewVoxelElementCount);

				ProxyData->PUArrays->NumElementsInActiveVoxelBuffer = NewVoxelElementCount;

			}
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
	FIntVector2 BufferSizes(
		LocalData->NumElementsInCoarseParticleBuffers, LocalData->NumElementsInActiveVoxelBuffer);
	PUData->Update(SystemInstance, LocalData);

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
		OutFunctions.Add(Sig);
	}
	/**
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ApplyParticleMassName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("FPPos")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FPEase")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FPEaseNew")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = SpawnParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("NumNewParticles")));
		// TODO: Forward information of where particles can spawn.
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = MoveParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("FPPos")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FPEase")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("FPPosNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FPEaseNew")));
		OutFunctions.Add(Sig);
	}
	*/
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ClearTableName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetVoxelPositionAndRoomName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("VoxelPosition")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
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
			FunctionInfo.DefinitionName == GetVoxelPositionAndRoomName ||
			FunctionInfo.DefinitionName == ClearTableName;
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

	// HashTable Shader Parameters
	ShaderParameters->HTIndexAndRoom		=	PUData.PUBuffers->HTIndexAndRoomBufferRef;
	ShaderParameters->HTVelocityAndMass		=	PUData.PUBuffers->HTVelocityAndMassBufferRef;
	ShaderParameters->HTMaxBound			=	PUData.PUBuffers->HTMaxBoundBufferRef;
	ShaderParameters->HTMinBound			=	PUData.PUBuffers->HTMinBoundBufferRef;

	ShaderParameters->HTOccupancy			=	PUData.PUBuffers->HTOccupancyBufferRef;
	ShaderParameters->TableSize				=	PUData.PUArrays->NumElementsInActiveVoxelBuffer;
}

// ---------- STATIC FUNCTIONALITY BELOW ----------

const float UParticleUpsamplingInterface::PACKING_RATIO = 0.67f;
FPUArrays* UParticleUpsamplingInterface::LocalData = new FPUArrays();

void UParticleUpsamplingInterface::SetCoarseParticles(TArray<FVector4f> PositionsAndRadius, TArray<FVector4f> VelocitiesAndMasses)
{
	LocalData->CPPositionsAndRadius.SetNumZeroed(PositionsAndRadius.Num());
	LocalData->CPPositionsAndRadius = PositionsAndRadius;

	LocalData->CPVelocitiesAndMasses.SetNumZeroed(VelocitiesAndMasses.Num());
	LocalData->CPVelocitiesAndMasses = VelocitiesAndMasses;
}

void UParticleUpsamplingInterface::SetActiveVoxelIndices(TArray<FVector4f> AVIs)
{
	LocalData->ActiveVoxelIndices.SetNumZeroed(AVIs.Num());
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
