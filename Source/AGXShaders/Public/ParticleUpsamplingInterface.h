#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "ParticleUpsamplingInterface.generated.h"

struct CoarseParticle
{
	CoarseParticle(FVector4 PositionAndScale, FVector Velocity, float Mass) { 
		PositionAndRadius = PositionAndScale;
		VelocityAndMass = FVector4(Velocity, Mass);
	};
	FVector4 PositionAndRadius;
	FVector4 VelocityAndMass;
};

// This struct are not used to push data, only for calculating bytes when allocating buffers.
struct VoxelEntry
{
	FIntVector4 IndexAndRoom;
	FVector4f PositionAndMass;
	FVector4f Velocity;
	FVector4f MinBound;
	FVector4f MaxBound;
	FVector4f InnerMinBound;
	FVector4f InnerMaxBound;
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

	template <typename T>
	FShaderResourceViewRHIRef InitSRVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	template <typename BufferType>
	void CreateInternalBuffer(
		FRHICommandListBase& RHICmdList, FReadBuffer& OutputBuffer, uint32 ElementCount);
	template <typename BufferType>
	void UpdateInternalBuffer(
		FRHICommandListBase& RHICmdList, const TArray<BufferType>& InputData,
		FReadBuffer& OutputBuffer);
	
	template <typename T>
	void UpdateSRVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, TArray<T> DataArray,
		FShaderResourceViewRHIRef& OutSRV);

	template <typename T>
	FUnorderedAccessViewRHIRef InitUAVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	FShaderResourceViewRHIRef ActiveVoxelIndicesBufferRef;
	FShaderResourceViewRHIRef CoarseParticleBufferRef;

	FUnorderedAccessViewRHIRef HashTableBufferRef;
	FUnorderedAccessViewRHIRef HashTableOccupancyBufferRef;

private:
	const static int INITIAL_VOXEL_BUFFER_SIZE;
	const static int INITIAL_COARSE_PARTICLE_BUFFER_SIZE;
};

/** Struct that contains the data that exists on the CPU */
struct FPUArrays
{
	FVector2f MousePos;
	FIntPoint ScreenSize;

	TArray<CoarseParticle> CoarseParticles;

	int Time = 0;
	int NumActiveVoxels = 0;
	float Upsampling = 0;
	float VoxelSize = 0;
	float FineParticleMass = 0;
	float FineParticleRadius = 0;
	float EaseStepSize = 0;
	float NominalRadius = 0;
	float TimeStep = 0;
	int TableSize = 0;

	void CopyFrom(const FPUArrays* Other) 
	{
		MousePos = Other->MousePos;
		ScreenSize = Other->ScreenSize;

		CoarseParticles.SetNumZeroed(Other->CoarseParticles.Num());
		CoarseParticles = Other->CoarseParticles;

		Time = Other->Time;
		NumActiveVoxels = Other->NumActiveVoxels;
		Upsampling = Other->Upsampling;
		VoxelSize = Other->VoxelSize;
		FineParticleMass = Other->FineParticleMass;
		FineParticleRadius = Other->FineParticleRadius;
		EaseStepSize = Other->EaseStepSize;
		NominalRadius = Other->NominalRadius;
		TimeStep = Other->TimeStep;
		TableSize = Other->TableSize;
	};
};

/** Struct containing all the data which this NDI stores */
struct FPUData
{
	void Init(FNiagaraSystemInstance* SystemInstance);
	void Update(FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherArray);
	void Release();

	FPUBuffers* PUBuffers = nullptr;
	FPUArrays* PUArrays = nullptr;

	bool bNeedsBufferResize = false;
};


struct FParticleUpsamplingProxy : public FNiagaraDataInterfaceProxy
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


UCLASS(EditInlineNew, Category = "Data Interface", CollapseCategories, meta = (DisplayName = "Particle Upsampling Interface"))
class AGXSHADERS_API UParticleUpsamplingInterface : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		// Particle Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FIntVector4>,		ActiveVoxelIndices)
		SHADER_PARAMETER_SRV(StructuredBuffer<CoarseParticle>,	CoarseParticles)
		SHADER_PARAMETER(int,									NumActiveVoxels)
		SHADER_PARAMETER(int,									NumCoarseParticles)
		SHADER_PARAMETER(int,									Time)
		SHADER_PARAMETER(float,									TimeStep)
		SHADER_PARAMETER(float,									VoxelSize)
		SHADER_PARAMETER(float,									FineParticleMass)
		SHADER_PARAMETER(float,									AnimationSpeed)
		SHADER_PARAMETER(float,									NominalRadius)
		// HashTable Buffers
		SHADER_PARAMETER_UAV(RWStructuredBuffer<VoxelEntry>,	HashTableBuffer)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<unsigned int>,	HashTableOccupancy)
		SHADER_PARAMETER(unsigned int,							TableSize)
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
	};
	virtual bool HasPostSimulateTick() const override
	{
		return true;
	};

	virtual bool PerInstanceTick(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool PerInstanceTickPostSimulate(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual void ProvidePerInstanceDataForRenderThread(
		void* DataForRenderThread, void* PerInstanceData,
		const FNiagaraSystemInstanceID& SystemInstance) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	// UNiagaraDataInterface Interface

	void GetMousePositionVM(FVectorVMExternalFunctionContext& Context);

	static void SetCoarseParticles(TArray<CoarseParticle> CPs);
	static void RecalculateFineParticleProperties(float Upsampling, float ElementSize, float ParticleDensity);
	static void SetStaticVariables(float Upsampling, float VoxelSize, float EaseStepSize);

protected:
#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(
		TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	/** Copy one niagara DI to this */
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

private:
	static FPUArrays* LocalData;

	static const FName GetMousePositionName;
	static const FName GetFineParticlePositionName;

	const static float PACKING_RATIO;
};