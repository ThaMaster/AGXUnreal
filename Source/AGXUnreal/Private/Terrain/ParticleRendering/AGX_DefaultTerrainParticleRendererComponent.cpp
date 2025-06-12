// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_DefaultTerrainParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

UAGX_DefaultTerrainParticleRendererComponent::UAGX_DefaultTerrainParticleRendererComponent()
{
	if (ParticleSystemAsset != nullptr)
		return;

	ParticleSystemAsset = FindNiagaraSystemAsset(NIAGARA_SYSTEM_PATH);
}

void UAGX_DefaultTerrainParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializeNiagaraParticleSystemComponent())
	{
		return;
	}

	// Attach lambda function to Function Delegate.
	// HandleParticleData will always run when AGX_Terrain updates particle data.
	DelegateHandle = ParentTerrainActor->UpdateParticleDataDelegate.AddLambda(
		[this](FParticleDataById data) { HandleParticleData(data); }
	);
}

void UAGX_DefaultTerrainParticleRendererComponent::HandleParticleData(FParticleDataById data)
{
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	const TArray<FVector>& Positions = data.Positions;
	const TArray<FQuat>& Rotations = data.Rotations;
	const TArray<float>& Radii = data.Radii;
	const TArray<bool>& Exists = data.Exists;
	const TArray<FVector>& Velocities = data.Velocities;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt("User.Target Particle Count", Exists.Num());
#else
	ParticleSystemComponent->SetVariableInt(FName("User.Target Particle Count"), Exists.Num());
#endif

	const int32 NumParticles = Positions.Num();

	TArray<FVector4> PositionsAndScale;
	PositionsAndScale.SetNum(NumParticles);
	TArray<FVector4> Orientations;
	Orientations.SetNum(NumParticles);

	for (int32 I = 0; I < NumParticles; ++I)
	{
		// The particle size slot in the PositionAndScale buffer is a scale and not the
		// actual size. The scale is relative to a SI unit cube, meaning that a
		// scale of 1.0 should render a particle that is 1x1x1 m large, or
		// 100x100x100 Unreal units. We multiply by 2.0 to convert from radius
		// to full width.
		float UnitCubeScale = (Radii[I] * 2.0f) / 100.0f;
		PositionsAndScale[I] = FVector4(Positions[I], UnitCubeScale);
		Orientations[I] = FVector4(Rotations[I].X, Rotations[I].Y, Rotations[I].Z, Rotations[I].W);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Positions And Scales", PositionsAndScale);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Orientations", Orientations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
		ParticleSystemComponent, "Exists", Exists);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		ParticleSystemComponent, TEXT("Velocities"), Velocities);
}