// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"

#include "AGX_ParticleUpsamplingDI.generated.h"

struct FPUArrays;
struct FCoarseParticle;

UCLASS(EditInlineNew, Category = "Data Interface", CollapseCategories, meta = (DisplayName = "Particle Upsampling Data Interface"))
class AGXUNREAL_API UAGX_ParticleUpsamplingDI : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		// Particle Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FCoarseParticle>,	CoarseParticles)
		SHADER_PARAMETER(int,									NumCoarseParticles)
		SHADER_PARAMETER(float,									FineParticleMass)
		SHADER_PARAMETER(float,									FineParticleRadius)
		SHADER_PARAMETER(float,									NominalRadius)

		// HashTable Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FIntVector4>,		ActiveVoxelIndices)
		SHADER_PARAMETER(int,									NumActiveVoxels)

		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVoxelEntry>,	HashTable)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>,			HTOccupancy)

		SHADER_PARAMETER(int,									TableSize)
		SHADER_PARAMETER(float,									VoxelSize)

		// Other Variables
		SHADER_PARAMETER(int,									Time)
		SHADER_PARAMETER(float,									AnimationSpeed)
	END_SHADER_PARAMETER_STRUCT()

public:
	virtual void PostInitProperties() override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override;
	virtual void BuildShaderParameters(
		FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
	virtual void SetShaderParameters(
		const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;
	virtual bool InitPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;

	virtual int32 PerInstanceDataSize() const override;
	virtual bool HasPreSimulateTick() const override;
	virtual bool HasPostSimulateTick() const override;

	virtual bool PerInstanceTick(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool PerInstanceTickPostSimulate(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual void ProvidePerInstanceDataForRenderThread(
		void* DataForRenderThread, void* PerInstanceData,
		const FNiagaraSystemInstanceID& SystemInstance) override;

	void SetCoarseParticles(TArray<FCoarseParticle> NewCoarseParticles);
	void SetActiveVoxelIndices(TArray<FIntVector4> AVIs);
	int GetHashTableSize();
	void RecalculateFineParticleProperties(float Upsampling, float ElementSize, float ParticleDensity);
	void SetStaticVariables(float VoxelSize, float EaseStepSize);

#if WITH_EDITORONLY_DATA
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
	virtual void GetParameterDefinitionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual bool GetFunctionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
		const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
		FString& OutHLSL) override;
#endif

protected:

#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(
		TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif

private:
	
	FPUArrays* LocalData;

	// Function names
	const FName UpdateGridName = TEXT("UpdateGrid");
	const FName ApplyParticleMassName = TEXT("ApplyParticleMass");
	const FName SpawnParticlesName = TEXT("SpawnParticles"); // Maybe change to get room?
	const FName MoveParticlesName = TEXT("MoveParticles");
	const FName ClearTableName = TEXT("ClearTable");
	const FName GetVoxelPositionAndRoomName = TEXT("GetVoxelPositionAndRoom");
	const FName GetNominalRadiusName = TEXT("GetNominalRadius");
	const FName GetFineParticleRadiusName = TEXT("GetFineParticleRadius");
	const FName IsFineParticleAliveName = TEXT("IsFineParticleAlive");
	const FName GetCoarseParticleInfoName = TEXT("GetCoarseParticleInfo");

	const TCHAR* PUUnrealShaderHeaderFile = TEXT("/ParticleRenderingShaders/ParticleUpsampling.ush");
	const float PACKING_RATIO = 0.67f;
};
