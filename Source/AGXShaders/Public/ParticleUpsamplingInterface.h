#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "ParticleUpsamplingInterface.generated.h"

struct FPUBuffers : public FRenderResource
{
	const int INITIAL_VOXEL_BUFFER_SIZE = 256;
	const int INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;

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
	template <typename T>
	void UpdateSRVBuffer(
		FRHICommandListBase& RHICmdList, const TArray<T>& InputData,
		FShaderResourceViewRHIRef& OutputBuffer);

	template <typename T>
	FUnorderedAccessViewRHIRef InitUAVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	FShaderResourceViewRHIRef ActiveVoxelIndicesBufferRef;
	FShaderResourceViewRHIRef CPPositionsAndRadiusBufferRef;
	FShaderResourceViewRHIRef CPVelocitiesAndMassesBufferRef;

	// HashTable RW Buffers
	FUnorderedAccessViewRHIRef HTIndexAndRoomBufferRef;
	FUnorderedAccessViewRHIRef HTOccupancyBufferRef;

	int HashTableSize = 0;
};

/** Struct that contains the data that exists on the CPU */
struct FPUArrays
{
	const int INITIAL_VOXEL_BUFFER_SIZE = 256;
	const int INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;

	TArray<FVector4f> CPPositionsAndRadius;
	TArray<FVector4f> CPVelocitiesAndMasses;
	TArray<FVector4f> ActiveVoxelIndices;

	int Time = 0;
	float VoxelSize = 0;
	float FineParticleMass = 0;
	float FineParticleRadius = 0;
	float EaseStepSize = 0;
	float NominalRadius = 0;
	float TimeStep = 0;
	int TableSize = 0;

	bool bCoarseParticlesBufferNeedsReisze = false;
	bool bActiveVoxelIndicesBufferNeedsResize = false;

	void CopyFrom(const FPUArrays* Other) 
	{
		CPPositionsAndRadius.SetNumZeroed(Other->CPPositionsAndRadius.Num());
		CPPositionsAndRadius = Other->CPPositionsAndRadius;

		CPVelocitiesAndMasses.SetNumZeroed(Other->CPVelocitiesAndMasses.Num());
		CPVelocitiesAndMasses = Other->CPVelocitiesAndMasses;

		ActiveVoxelIndices.SetNumZeroed(Other->ActiveVoxelIndices.Num());
		ActiveVoxelIndices = Other->ActiveVoxelIndices;

		Time = Other->Time;
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
	void Update(FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherData);
	void Release();

	FPUBuffers* PUBuffers = nullptr;
	FPUArrays* PUArrays = nullptr;
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
		SHADER_PARAMETER_SRV(StructuredBuffer<FVector4f>,		ActiveVoxelIndices)
		SHADER_PARAMETER_SRV(StructuredBuffer<FVector4f>,		CPPositionsAndRadius)
		SHADER_PARAMETER_SRV(StructuredBuffer<FVector4f>,		CPVelocitiesAndMasses)
		SHADER_PARAMETER(int,									NumActiveVoxels)
		SHADER_PARAMETER(int,									NumCoarseParticles)
		SHADER_PARAMETER(float,									VoxelSize)
		SHADER_PARAMETER(float,									FineParticleMass)
		SHADER_PARAMETER(float,									FineParticleRadius)
		SHADER_PARAMETER(float,									NominalRadius)
		
		// HashTable Buffers
		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector4f>,	HTIndexAndRoom)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>,		HTOccupancy)
		SHADER_PARAMETER(int,								TableSize)
	END_SHADER_PARAMETER_STRUCT()

public:

	// Runs once to set up the data interface
	virtual void PostInitProperties() override;

	// UNiagaraDataInterface Interface
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
	// UNiagaraDataInterface Interface

	static void SetCoarseParticles(
		TArray<FVector4f> PositionsAndRadius, TArray<FVector4f> VelocitiesAndMasses);
	static void SetActiveVoxelIndices(TArray<FVector4f> AVIs);

	static void RecalculateFineParticleProperties(float Upsampling, float ElementSize, float ParticleDensity);
	static void SetStaticVariables(float VoxelSize, float EaseStepSize);

protected:
#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(
		TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
private:
	static FPUArrays* LocalData;

	static const FName GetFineParticlePositionAndRadiusName;
	static const FName GetFineParticleVelocityAndMassName;
	static const FName GetNumCoarseParticlesName;
	static const FName GetActiveVoxelIndexName;
	static const FName GetFineParticleRadiusName;
	static const FName UpdateGridName;
	static const FName LookupRoomName;
	static const FName InsertIndexName;
	const static float PACKING_RATIO;
};