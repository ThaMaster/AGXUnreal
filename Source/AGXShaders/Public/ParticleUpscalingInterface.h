#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"

#include "NiagaraDataInterface.h"

#include "NiagaraDataInterfaceRW.h"
#include "ParticleUpscalingInterface.generated.h"

struct CoarseParticle
{
	FVector4f PositionAndRadius;
	FVector4f VelocityAndMass;
};

struct FineParticle
{
	FVector4f PositionAndEase;
	FVector4f VelocityAndMass;
};

struct FPUBuffers : public FRenderResource
{
	/** Init the buffer */
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	/** Release the buffer */
	virtual void ReleaseRHI() override;

	/** Get the resource name */
	virtual FString GetFriendlyName() const override
	{
		return TEXT("FPUBuffers");
	}

	FUnorderedAccessViewRHIRef CoarseParticleBufferRef;
};

struct FPUArrays
{
	FVector2f MousePos;
	FIntPoint ScreenSize;

	TArray<CoarseParticle> CoarseParticles;
	TArray<FineParticle> FineParticles;
};

struct FPUData
{
	void Init(FNiagaraSystemInstance* SystemInstance);
	void Update(FNiagaraSystemInstance* SystemInstance);
	void Release();

	FPUBuffers* PUBuffers = nullptr;
	FPUArrays* PUArrays = nullptr;

	bool bNeedsBufferResize = false;
};


struct FParticleUpscalingProxy : public FNiagaraDataInterfaceProxy
{
	/** Get the size of the data that will be passed to render*/
	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override
	{
		return sizeof(FPUData);
	}

	/** Get the data that will be passed to render*/
	virtual void ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID) override;

	/** Initialize the Proxy data buffer */
	void InitializePerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

	/** Destroy the proxy data if necessary */
	void DestroyPerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

	/** Launch all pre stage functions */
	virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;

	/** List of proxy data for each system instances*/
	TMap<FNiagaraSystemInstanceID, FPUData> SystemInstancesToInstanceData_RT;
};


UCLASS(EditInlineNew, Category = "Data Interface", CollapseCategories, meta = (DisplayName = "Particle Upscaling Interface"))
class AGXSHADERS_API UParticleUpscalingInterface : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		SHADER_PARAMETER_UAV(RWStructuredBuffer<CoarseParticle>, CoarseParticles)
	END_SHADER_PARAMETER_STRUCT()
public:
	// Runs once to set up the data interface
	virtual void PostInitProperties() override;

	// UNiagaraDataInterface Interface
	virtual void GetVMExternalFunction(
		const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData,
		FVMExternalFunction& OutFunc) override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override
	{
		return true;
	}
#if WITH_EDITORONLY_DATA
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
	virtual void GetParameterDefinitionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
		const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
		FString& OutHLSL) override;
#endif
	virtual void BuildShaderParameters(
		FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(
		const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;
	virtual bool InitPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override
	{
		return sizeof(FPUData);
	};
	virtual bool HasPreSimulateTick() const override
	{
		return true;
	}
	virtual bool PerInstanceTick(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual void ProvidePerInstanceDataForRenderThread(
		void* DataForRenderThread, void* PerInstanceData,
		const FNiagaraSystemInstanceID& SystemInstance) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	// UNiagaraDataInterface Interface

	void GetMousePositionVM(FVectorVMExternalFunctionContext& Context);

protected:
#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(
		TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	/** Copy one niagara DI to this */
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

private:
	static const FName GetMousePositionName;
};