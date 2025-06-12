// Copyright 2025, Algoryx Simulation AB.
#include "Terrain/ParticleRendering/AGX_TerrainParticleUpsamplerRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/NiagaraDataInterfaces/AGX_ParticleUpsamplingInterface.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

UAGX_TerrainParticleUpsamplerRendererComponent::UAGX_TerrainParticleUpsamplerRendererComponent()
{
	if (ParticleSystemAsset != nullptr)
		return;

	ParticleSystemAsset = FindNiagaraSystemAsset(NIAGARA_UPSAMPLING_SYSTEM_PATH);
}

void UAGX_TerrainParticleUpsamplerRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InitializeNiagaraParticleSystemComponent())
	{
		return;
	}

	// Attach lambda function to Function Delegate.
	// HandleParticleData will always run when AGX_Terrain updates particle data.
	DelegateHandle = ParentTerrainActor->UpdateParticleDataDelegate.AddLambda(
		[this](FParticleDataById data) { HandleParticleData(data); });
}

void UAGX_TerrainParticleUpsamplerRendererComponent::HandleParticleData(FParticleDataById data)
{
	const TArray<FVector>& Positions = data.Positions;
	const TArray<FQuat>& Rotations = data.Rotations;
	const TArray<float>& Radii = data.Radii;
	const TArray<bool>& Exists = data.Exists;
	const TArray<FVector>& Velocities = data.Velocities;
	const int32 NumParticles = Positions.Num();

	// TODO: ADD MASSES TO PARTICLE DATA
	//const TArray<float>& Masses;


	TArray<FVector4> PositionsAndScale;
	PositionsAndScale.SetNum(NumParticles);
	TArray<FVector4> Orientations;
	Orientations.SetNum(NumParticles);
	TArray<FCoarseParticle> NewCoarseParticles;
	TSet<FIntVector> ActiveVoxelSet;
	float ParticleDensity = 0.0f;
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
		if (Exists[I] && bEnableParticleRendering)
		{
			float Mass = 1;
			//float Mass = Masses[I]; // Convert to gram
			float Volume = (4.0 / 3.0) * PI * FMath::Pow(Radii[I], 3);
			ParticleDensity = Mass / Volume;
			AppendIfActiveVoxel(ActiveVoxelSet, Positions[I], Radii[I]);
			FCoarseParticle CP;
			CP.PositionAndRadius =
				FVector4f(Positions[I].X, Positions[I].Y, Positions[I].Z, Radii[I]);
			CP.VelocityAndMass =
				FVector4f(Velocities[I].X, Velocities[I].Y, Velocities[I].Z, 1); // TODO CHANGE THIS
			NewCoarseParticles.Add(CP);
		}
	}
	if (ParticleDensity > 0.0f)
	{
		// This is the element size in the unity version, only to be used when comparing
		// implementations / for thesis work.
		const float ElementSize = 15.0f;
		TArray<FIntVector4> ActiveVoxelIndices = GetActiveVoxelsFromSet(ActiveVoxelSet);
		//UParticleUpsamplingInterface::SetCoarseParticles(NewCoarseParticles);
		//UParticleUpsamplingInterface::SetActiveVoxelIndices(ActiveVoxelIndices);
		//UParticleUpsamplingInterface::RecalculateFineParticleProperties(
		//	Upsampling, ElementSize, ParticleDensity);
		//UParticleUpsamplingInterface::SetStaticVariables(VoxelSize, EaseStepSize);
		//int HashTableSize = UParticleUpsamplingInterface::GetHashTableSize();
#if UE_VERSION_OLDER_THAN(5, 3, 0)
		ParticleSystemComponent->SetNiagaraVariableInt(
			"User.Active Voxels Count", ActiveVoxelIndices.Num());
		ParticleSystemComponent->SetNiagaraVariableInt("User.HashTable Size", HashTableSize);
		ParticleSystemComponent->SetNiagaraVariableFloat("User.Voxel Size", VoxelSize);
#else
		ParticleSystemComponent->SetVariableInt(
			FName("User.Active Voxels Count"), ActiveVoxelIndices.Num());
		//ParticleSystemComponent->SetVariableInt(FName("User.HashTable Size"), HashTableSize);
		ParticleSystemComponent->SetVariableFloat(FName("User.Voxel Size"), VoxelSize);
#endif
	}
}

void UAGX_TerrainParticleUpsamplerRendererComponent::AppendIfActiveVoxel(
	TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius)
{
	float AABBRadius = 1.6119919540164696407169668466392849389446140723238615 * CPRadius / 2;
	FVector VSPosition = CPPosition / VoxelSize;

	FVector OffsetPosition(
		FMath::Sign(VSPosition.X), FMath::Sign(VSPosition.Y), FMath::Sign(VSPosition.Z));

	OffsetPosition *= 0.5;
	OffsetPosition += VSPosition;

	FVector VSParticleVoxelPos(
		(double) (int) OffsetPosition.X, (double) (int) OffsetPosition.Y,
		(double) (int) OffsetPosition.Z);

	int n = (int) (AABBRadius / VoxelSize + 1);
	for (int x = -n; x <= n; x++)
	{
		for (int y = -n; y <= n; y++)
		{
			for (int z = -n; z <= n; z++)
			{
				FVector VSVoxelPos = VSParticleVoxelPos + FVector(x, y, z);
				FVector VSVoxelToParticle = VSPosition - VSVoxelPos;

				FVector VSToEdge =
					FVector::Max(VSVoxelToParticle, -VSVoxelToParticle) - AABBRadius / VoxelSize;
				FVector VSVoxelSize2(0.5);
				if (FVector::Max(VSToEdge, VSVoxelSize2) == VSVoxelSize2)
				{
					VSVoxelPos += FVector(
									  FMath::Sign(VSVoxelPos.X), FMath::Sign(VSVoxelPos.Y),
									  FMath::Sign(VSVoxelPos.Z)) *
								  0.5;
					ActiveVoxelIndices.Add(FIntVector(VSVoxelPos));
				}
			}
		}
	}
}

TArray<FIntVector4> UAGX_TerrainParticleUpsamplerRendererComponent::GetActiveVoxelsFromSet(
	TSet<FIntVector> VoxelSet)
{
	TArray<FIntVector4> ActiveVoxels;
	for (FIntVector Voxel : VoxelSet)
	{
		ActiveVoxels.Add(FIntVector4(Voxel, 0));
	}
	return ActiveVoxels;
}
