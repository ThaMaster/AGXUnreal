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

#if WITH_EDITORONLY_DATA
#include "LevelEditorViewport.h"
#else
#include "Engine/World.h"
#endif
#include "GameFramework/PlayerController.h" // REMOVE THIS LATER

#define LOCTEXT_NAMESPACE "NDIParticleUpsampling"

const FName UParticleUpsamplingInterface::GetMousePositionName(TEXT("GetMousePosition"));
const FName UParticleUpsamplingInterface::GetFineParticlePositionName(TEXT("GetFineParticlePosition"));
const FName UParticleUpsamplingInterface::GetNumCoarseParticlesName(TEXT("GetNumCoarseParticles"));
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
const int FPUBuffers::INITIAL_VOXEL_BUFFER_SIZE = 128;
const int FPUBuffers::INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;


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
void FPUBuffers::UpdateSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, TArray<T> DataArray,
	FShaderResourceViewRHIRef& OutSRV)
{
	OutSRV.SafeRelease();

	FResourceArrayUploadArrayView ResourceData(DataArray.GetData(), sizeof(T) * DataArray.Num());
	FRHIResourceCreateInfo CreateInfo(InDebugName, &ResourceData);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * DataArray.Num(), BUF_ShaderResource, CreateInfo);

	OutSRV = RHICmdList.CreateShaderResourceView(
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

template <typename BufferType>
void FPUBuffers::CreateInternalBuffer(
	FRHICommandListBase& RHICmdList, FReadBuffer& OutputBuffer, uint32 ElementCount)
{
	if (ElementCount > 0)
	{
		OutputBuffer.Initialize(RHICmdList, TEXT("ParticleUpsamplingBuffer"), sizeof(BufferType), ElementCount, EPixelFormat::PF_A32B32G32R32F, BUF_StructuredBuffer);
	}
}

template <typename BufferType>
void FPUBuffers::UpdateInternalBuffer(
	FRHICommandListBase& RHICmdList, const TArray<BufferType>& InputData, FReadBuffer& OutputBuffer)
{
	uint32 ElementCount = InputData.Num();
	if (ElementCount > 0 && OutputBuffer.Buffer.IsValid())
	{
		const uint32 BufferBytes = sizeof(BufferType) * ElementCount;

		void* OutputData =
			RHICmdList.LockBuffer(OutputBuffer.Buffer, 0, BufferBytes, RLM_WriteOnly);

		FMemory::Memcpy(OutputData, InputData.GetData(), BufferBytes);
		RHICmdList.UnlockBuffer(OutputBuffer.Buffer);
	}
}

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Particle Buffers
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), INITIAL_VOXEL_BUFFER_SIZE);
	CoarseParticleBufferRef = InitSRVBuffer<CoarseParticle>(
		RHICmdList, TEXT("CoarseParticleBufferRef"), INITIAL_COARSE_PARTICLE_BUFFER_SIZE);

	// HashTable Buffers
	HashTableBufferRef =InitUAVBuffer<VoxelEntry>(
		RHICmdList, TEXT("HashTableBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
	HashTableOccupancyBufferRef = InitUAVBuffer<unsigned int>(
		RHICmdList, TEXT("HashTableOccupancyBuffer"), INITIAL_VOXEL_BUFFER_SIZE * 2);
}

void FPUBuffers::ReleaseRHI()
{
	// Particle Buffers
	ActiveVoxelIndicesBufferRef.SafeRelease();
	CoarseParticleBufferRef.SafeRelease();

	// HashTable Buffers
	HashTableBufferRef.SafeRelease();
	HashTableOccupancyBufferRef.SafeRelease();
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
		PUArrays->MousePos = FVector2f::ZeroVector;
		PUBuffers = new FPUBuffers();
		BeginInitResource(PUBuffers);
	}
}

void FPUData::Update(FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherArray)
{
	if (SystemInstance)
	{
		PUArrays->CopyFrom(OtherArray);
		PUArrays->MousePos = FVector2f::ZeroVector;
		PUArrays->TimeStep = SystemInstance->GetLastRenderTime();
		PUArrays->Time = (int) std::time(0);

		// If we have a player controller we use it to capture the mouse position
		UWorld* World = SystemInstance->GetWorld();
		if (World && World->GetNumPlayerControllers() > 0)
		{
			APlayerController* Controller = World->GetFirstPlayerController();
			Controller->GetMousePosition(PUArrays->MousePos.X, PUArrays->MousePos.Y);
			Controller->GetViewportSize(PUArrays->ScreenSize.X, PUArrays->ScreenSize.Y);
			return;
		}

#if WITH_EDITORONLY_DATA
		// While in the editor we don't necessarily have a player controller, so we query the viewport
		// object instead
		if (GCurrentLevelEditingViewportClient)
		{
			PUArrays->MousePos.X = GCurrentLevelEditingViewportClient->Viewport->GetMouseX();
			PUArrays->MousePos.Y = GCurrentLevelEditingViewportClient->Viewport->GetMouseY();
			PUArrays->ScreenSize = GCurrentLevelEditingViewportClient->Viewport->GetSizeXY();
		}
#endif
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

void FParticleUpsamplingProxy::DestroyPerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
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
		if (Context.GetSimStageData().bFirstStage && ProxyData->PUArrays->CoarseParticles.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();
			UE_LOG(
				LogTemp, Warning, TEXT("[PreStage] PosX: %f, PosY: %f, PosZ: %f"),
				ProxyData->PUArrays->CoarseParticles[0].PositionAndRadius.X,
				ProxyData->PUArrays->CoarseParticles[0].PositionAndRadius.Y,
				ProxyData->PUArrays->CoarseParticles[0].PositionAndRadius.Z);
			ProxyData->PUBuffers->UpdateSRVBuffer<CoarseParticle>(
				RHICmdList, TEXT("CoarseParticleBufferRef"), ProxyData->PUArrays->CoarseParticles,
				ProxyData->PUBuffers->CoarseParticleBufferRef);
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

UParticleUpsamplingInterface::UParticleUpsamplingInterface(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpsamplingProxy());
	LocalData = new FPUArrays();
}

bool UParticleUpsamplingInterface::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* PUData = new (PerInstanceData) FPUData;
	PUData->Init(SystemInstance);
	return true;
}

// clean up RT instances
void UParticleUpsamplingInterface::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* InstanceData =
		static_cast<FPUData*>(PerInstanceData);
	InstanceData->~FPUData();

	ENQUEUE_RENDER_COMMAND(RemoveProxy)
	([RT_Proxy = GetProxyAs<FParticleUpsamplingProxy>(), 
		InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{ 
			RT_Proxy->SystemInstancesToInstanceData_RT.Remove(InstanceID); 
		});
}

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
	return false;
}

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

// this registers our custom DI with Niagara
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
// this lists all the functions our DI provides (currently only one)
void UParticleUpsamplingInterface::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetMousePositionName;
		Sig.Description = LOCTEXT(
			"GetMousePositionNameFunctionDescription",
			"Returns the mouse position in screen space.");
		Sig.bMemberFunction = true;
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Normalized")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("PosX")),
			LOCTEXT(
				"MousePosXDescription",
				"Returns the x coordinates in pixels or 0-1 range if normalized"));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("PosY")),
			LOCTEXT(
				"MousePosYDescription",
				"Returns the y coordinates in pixels or 0-1 range if normalized"));
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetFineParticlePositionName;
		Sig.bMemberFunction = true;
		Sig.AddInput(
			FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Index")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("FineParticlePosition")));
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
}
#endif

// this provides the cpu vm with the correct function to call
DEFINE_NDI_DIRECT_FUNC_BINDER(UParticleUpsamplingInterface, GetMousePositionVM);

void UParticleUpsamplingInterface::GetVMExternalFunction(
	const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData,
	FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetMousePositionName)
	{
		NDI_FUNC_BINDER(UParticleUpsamplingInterface, GetMousePositionVM)::Bind(this, OutFunc);
	}
	else
	{
		UE_LOG(
			LogTemp, Display,
			TEXT("Could not find data interface external function in %s. Received Name: %s"),
			*GetPathNameSafe(this), *BindingInfo.Name.ToString());
	}
}

// implementation called by the vectorVM
void UParticleUpsamplingInterface::GetMousePositionVM(
	FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FPUData> InstData(Context);
	FNDIInputParam<bool> InNormalized(Context);
	FNDIOutputParam<float> OutPosX(Context);
	FNDIOutputParam<float> OutPosY(Context);

	FVector2f MousePos = InstData.Get()->PUArrays->MousePos;
	FIntPoint ScreenSize = InstData.Get()->PUArrays->ScreenSize;

	// iterate over the particles
	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		if (InNormalized.GetAndAdvance())
		{
			OutPosX.SetAndAdvance(MousePos.X / ScreenSize.X);
			OutPosY.SetAndAdvance(MousePos.Y / ScreenSize.Y);
		}
		else
		{
			OutPosX.SetAndAdvance(MousePos.X);
			OutPosY.SetAndAdvance(MousePos.Y);
		}
	}
}

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

// This can be used to provide the hlsl code for gpu scripts. If the DI supports only cpu
// implementations, this is not needed. We don't need to actually print our function code to OutHLSL
// here because we use a template file that gets appended in GetParameterDefinitionHLSL(). If the
// hlsl function is so simple that it does not need bound shader parameters, then this method can be
// used instead of GetParameterDefinitionHLSL.
bool UParticleUpsamplingInterface::GetFunctionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
	FString& OutHLSL)
{
	return FunctionInfo.DefinitionName == GetMousePositionName || FunctionInfo.DefinitionName == GetFineParticlePositionName || FunctionInfo.DefinitionName == GetNumCoarseParticlesName;
}

// this loads our hlsl template script file and
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
	//ShaderParameters->ActiveVoxelIndices =		PUData.PUBuffers->ActiveVoxelIndicesBufferRef;
	//ShaderParameters->CoarseParticles =			PUData.PUBuffers->CoarseParticleBufferRef;

	//ShaderParameters->NumActiveVoxels =			PUData.PUArrays->NumActiveVoxels;
	ShaderParameters->NumCoarseParticles =		PUData.PUArrays->CoarseParticles.Num();
	//ShaderParameters->Time =					PUData.PUArrays->Time;
	//ShaderParameters->TimeStep =				PUData.PUArrays->TimeStep;
	//ShaderParameters->VoxelSize =				PUData.PUArrays->VoxelSize;
	//ShaderParameters->FineParticleMass =		PUData.PUArrays->FineParticleMass;
	//ShaderParameters->AnimationSpeed =			PUData.PUArrays->EaseStepSize;
	//ShaderParameters->NominalRadius =			PUData.PUArrays->NominalRadius;

	// HashTable Shader Parameters
	//ShaderParameters->HashTableBuffer =			PUData.PUBuffers->HashTableBufferRef;
	//ShaderParameters->HashTableOccupancy =		PUData.PUBuffers->HashTableOccupancyBufferRef;
	//ShaderParameters->TableSize =				PUData.PUArrays->TableSize;
}

bool UParticleUpsamplingInterface::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UParticleUpsamplingInterface* OtherTyped =
		CastChecked<const UParticleUpsamplingInterface>(Other);

	// COMPARE OTHER VARIABLES BELOW!
	return true;
}

bool UParticleUpsamplingInterface::CopyToInternal(
	UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}

	UParticleUpsamplingInterface* OtherTyped = CastChecked<UParticleUpsamplingInterface>(Destination);

	// SET OTHER VARIABLES BEFORE RETURNING!
	return true;
}


// ---------- STATIC FUNCTIONALITY BELOW ----------

const float UParticleUpsamplingInterface::PACKING_RATIO = 0.67f;
FPUArrays* UParticleUpsamplingInterface::LocalData = new FPUArrays();

void UParticleUpsamplingInterface::SetCoarseParticles(TArray<CoarseParticle> CPs)
{
	LocalData->CoarseParticles = CPs;
}

void UParticleUpsamplingInterface::RecalculateFineParticleProperties(
	float Upsampling, float ElementSize, float ParticleDensity)
{
	LocalData->NominalRadius =
		FMath::Pow(3.0f * PACKING_RATIO / (4.0f * PI), 1.0f / 3.0f) * ElementSize;
	LocalData->FineParticleRadius = LocalData->NominalRadius / FMath::Pow(Upsampling, 1.0f / 3.0f);
	float NominalMass =
		ParticleDensity * 4.0f / 3.0f * PI * FMath::Pow(LocalData->NominalRadius, 3.0f);
	LocalData->FineParticleMass = NominalMass / Upsampling;
}


#undef LOCTEXT_NAMESPACE
