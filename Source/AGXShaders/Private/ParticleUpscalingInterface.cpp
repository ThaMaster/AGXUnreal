#include "ParticleUpscalingInterface.h"
#include "NiagaraCompileHashVisitor.h"
#include "NiagaraTypes.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraShaderParametersBuilder.h"
#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "RHIUtilities.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "NiagaraSimStageData.h"

#if WITH_EDITORONLY_DATA
#include "LevelEditorViewport.h"
#else
#include "Engine/World.h"
#endif
#include "GameFramework/PlayerController.h" // REMOVE THIS LATER

#define LOCTEXT_NAMESPACE "NDIParticleUpscaling"

const FName UParticleUpscalingInterface::GetMousePositionName(TEXT("GetMousePosition"));

static const TCHAR* ParticleUpscalingTemplateShaderFile = TEXT("/AGXShadersShaders/ParticleUpscaling.ush");

// CoarseParticleBuffer -> Fås av CPUn varje tidssteg
// FineParticleBuffer -> Statisk, persistant, stor nog många bytes, byggs up i GPUn. (Se hur de hanteras i Unity/c#).
// ActiveVoxels -> Buffer/Lista fås av CPUn varje tidsteg, AGX (kanske inte finns men skulle vara pog att ha)
// ActiveVoxel Hashmappen -> byggs upp utifrån ActiveVoxel Buffern som skickas från CPUn

/**
* Hur arbetet ska gå nu:
* 
* Tänk om hur detta ska fungera, istället för att skapa en custom NDI som är en generisk hashmap, gör istället denna till en Particle Upscaling klass (men i form av en NDI) som kan köra några funktioner på GPUn.
* Dessutom så kommer man inte behöva massa inputs i niagara vfx system editorn, utan istället sker det mesta automatiskt och man fetchar bara data från editorn istället. 
* 
* Se sedan över hur man ska ange trådar för compute shaders, eller om det blir tillräcklig bra prestanda!
* 
* Därefter, gör funktioner sådant att man kan skicka in en CoarseParticleBuffer och ActiveVoxelBuffer!
* 
* Det kommer bli mycket programmeing i .ush filer (tror jag)
* Note: Unity hanterar detta på ett sätt som verkligen liknar hur det ska hanteras i Unreal!
*/

// --------------------------------------------------------------- //
// ------------------------- DATA BUFFERS ------------------------ //
// --------------------------------------------------------------- //

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	TArray<float> MyDataArray;
	MyDataArray.SetNumZeroed(1);
	FResourceArrayUploadArrayView ResourceData(
		MyDataArray.GetData(), sizeof(FVector4f) * MyDataArray.Num());

	FRHIResourceCreateInfo CreateInfo(TEXT("StorageBuffer"), &ResourceData);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(FVector4f), sizeof(FVector4f) * MyDataArray.Num(), BUF_UnorderedAccess, CreateInfo);

	StorageBuffer = RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
}

void FPUBuffers::ReleaseRHI()
{
	StorageBuffer.SafeRelease();
}

// --------------------------------------------------------------- //
// ------------------------- DATA ARRAYS ------------------------- //
// --------------------------------------------------------------- //

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

void FPUData::Update(FNiagaraSystemInstance* SystemInstance)
{
	PUArrays->MousePos = FVector2f::ZeroVector;

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

void FParticleUpscalingProxy::InitializePerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());
	check(!SystemInstancesToInstanceData_RT.Contains(SystemInstance));

	SystemInstancesToInstanceData_RT.Add(SystemInstance);
}

void FParticleUpscalingProxy::DestroyPerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());

	SystemInstancesToInstanceData_RT.Remove(SystemInstance);
}

/** Update the buffers with the arrays here! */
void FParticleUpscalingProxy::PreStage(const FNDIGpuComputePreStageContext& Context)
{
	check(SystemInstancesToInstanceData_RT.Contains(Context.GetSystemInstanceID()));

	FPUData* ProxyData = SystemInstancesToInstanceData_RT.Find(Context.GetSystemInstanceID());
	if (ProxyData != nullptr && ProxyData->PUBuffers)
	{
		if (Context.GetSimStageData().bFirstStage)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();
			FVector2f MP = ProxyData->PUArrays->MousePos;
			FIntPoint SS = ProxyData->PUArrays->ScreenSize;

			const TArray<FVector4f> Data = {FVector4f(MP.X, MP.Y, static_cast<float>(SS.X), static_cast<float>(SS.Y))};

			const uint32 BufferBytes = sizeof(FVector4f) * Data.Num();

			// Copy data to the buffer
			void* OutputData = RHICmdList.LockBuffer(
				ProxyData->PUBuffers->StorageBuffer->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

			FMemory::Memcpy(OutputData, ProxyData->PUArrays, BufferBytes);
			RHICmdList.UnlockBuffer(ProxyData->PUBuffers->StorageBuffer->GetBuffer());
		}
	}
}

void FParticleUpscalingProxy::ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FPUData* InstanceDataFromGT = static_cast<FPUData*>(PerInstanceData);
	FPUData& InstanceData = SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData = *InstanceDataFromGT;

	// we call the destructor here to clean up the GT data. Without this we could be leaking
	// memory.
	InstanceDataFromGT->~FPUData();
}

// --------------------------------------------------------------- //
// ---------------- PARTICLE UPSCALING INTERFACE ----------------- //
// --------------------------------------------------------------- //

UParticleUpscalingInterface::UParticleUpscalingInterface(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpscalingProxy());
}


// creates a new data object to store our position in.
// Don't keep transient data on the data interface object itself, only use per instance data!
/**
* INITIERA ALLA BUFFRAR HÄR!
*/
bool UParticleUpscalingInterface::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* PUData = new (PerInstanceData) FPUData;
	PUData->Init(SystemInstance);
	return true;
}

// clean up RT instances
void UParticleUpscalingInterface::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FPUData* InstanceData =
		static_cast<FPUData*>(PerInstanceData);
	InstanceData->~FPUData();

	ENQUEUE_RENDER_COMMAND(RemoveProxy)
	([RT_Proxy = GetProxyAs<FParticleUpscalingProxy>(), 
		InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{ 
			RT_Proxy->SystemInstancesToInstanceData_RT.Remove(InstanceID); 
		});
}

/**
* 
* UPPDATERA BUFFRARNA MED DATAN FRÅN CPUn!
*/
// This ticks on the game thread and lets us do work to initialize the instance data.
// If you need to do work on the gathered instance data after the simulation is done, use
// PerInstanceTickPostSimulate() instead.
bool UParticleUpscalingInterface::PerInstanceTick(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FPUData* PUData =
		static_cast<FPUData*>(PerInstanceData);
	if (!PUData)
	{
		return true;
	}
	PUData->Update(SystemInstance);

	return false;
}

void UParticleUpscalingInterface::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	FPUData* RenderThreadData = static_cast<FPUData*>(DataForRenderThread);
	FPUData* GameThreadData = static_cast<FPUData*>(PerInstanceData);

	if (RenderThreadData && GameThreadData)
	{
		// Copy data from game thread to render thread
		*RenderThreadData = *GameThreadData;
	}
}

// this registers our custom DI with Niagara
void UParticleUpscalingInterface::PostInitProperties()
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
void UParticleUpscalingInterface::GetFunctionsInternal(
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
			FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("MousePosition interface")));
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
}
#endif

// this provides the cpu vm with the correct function to call
DEFINE_NDI_DIRECT_FUNC_BINDER(UParticleUpscalingInterface, GetMousePositionVM);

void UParticleUpscalingInterface::GetVMExternalFunction(
	const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData,
	FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetMousePositionName)
	{
		NDI_FUNC_BINDER(UParticleUpscalingInterface, GetMousePositionVM)::Bind(this, OutFunc);
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
void UParticleUpscalingInterface::GetMousePositionVM(
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
bool UParticleUpscalingInterface::AppendCompileHash(
	FNiagaraCompileHashVisitor* InVisitor) const
{
	bool bSuccess = Super::AppendCompileHash(InVisitor);
	bSuccess &= InVisitor->UpdateShaderFile(ParticleUpscalingTemplateShaderFile);
	bSuccess &= InVisitor->UpdateShaderParameters<FShaderParameters>();
	return bSuccess;
}

// This can be used to provide the hlsl code for gpu scripts. If the DI supports only cpu
// implementations, this is not needed. We don't need to actually print our function code to OutHLSL
// here because we use a template file that gets appended in GetParameterDefinitionHLSL(). If the
// hlsl function is so simple that it does not need bound shader parameters, then this method can be
// used instead of GetParameterDefinitionHLSL.
bool UParticleUpscalingInterface::GetFunctionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
	FString& OutHLSL)
{
	return FunctionInfo.DefinitionName == GetMousePositionName;
}

// this loads our hlsl template script file and
void UParticleUpscalingInterface::GetParameterDefinitionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	const TMap<FString, FStringFormatArg> TemplateArgs = {
		{TEXT("ParameterName"), ParamInfo.DataInterfaceHLSLSymbol},
	};
	AppendTemplateHLSL(OutHLSL, ParticleUpscalingTemplateShaderFile, TemplateArgs);
}

#endif

// This fills in the expected parameter bindings we use to send data to the GPU
void UParticleUpscalingInterface::BuildShaderParameters(
	FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

// This fills in the parameters to send to the GPU
void UParticleUpscalingInterface::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FParticleUpscalingProxy& DataInterfaceProxy = Context.GetProxy<FParticleUpscalingProxy>();
	FPUData& PUData = DataInterfaceProxy.SystemInstancesToInstanceData_RT.FindChecked(Context.GetSystemInstanceID());

	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();
	ShaderParameters->StorageBuffer = PUData.PUBuffers->StorageBuffer;
}

bool UParticleUpscalingInterface::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}
	const UParticleUpscalingInterface* OtherTyped =
		CastChecked<const UParticleUpscalingInterface>(Other);

	// COMPARE OTHER VARIABLES BELOW!
	return true;
}

bool UParticleUpscalingInterface::CopyToInternal(
	UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}

	UParticleUpscalingInterface* OtherTyped = CastChecked<UParticleUpscalingInterface>(Destination);

	// SET OTHER VARIABLES BEFORE RETURNING!
	return true;
}

#undef LOCTEXT_NAMESPACE
