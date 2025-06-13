// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_UpsamplingParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

/**
 *
 */
UAGX_UpsamplingParticleRendererComponent::UAGX_UpsamplingParticleRendererComponent()
{
	if (ParticleSystemAsset != nullptr)
		return;

	ParticleSystemAsset = FindNiagaraSystemAsset(NIAGARA_SYSTEM_PATH);
}

void UAGX_UpsamplingParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!bEnableParticleRendering)
	{
		return;
	}

	if (!ParentTerrainActor)
	{
		// Fetch the parent terrain actor.
		if (!InitializeParentTerrainActor())
		{
			return;
		}
	}

	if (!InitializeNiagaraParticleSystemComponent())
	{
		return;
	}

	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->UpdateParticleDataDelegate.AddDynamic(
		this, &UAGX_UpsamplingParticleRendererComponent::HandleParticleData);
}


void UAGX_UpsamplingParticleRendererComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ParentTerrainActor->UpdateParticleDataDelegate.RemoveDynamic(
		this, &UAGX_UpsamplingParticleRendererComponent::HandleParticleData);
}

bool UAGX_UpsamplingParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	return bEnableParticleRendering = bEnabled; 
}

bool UAGX_UpsamplingParticleRendererComponent::GetEnableParticleRendering()
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_UpsamplingParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

int32 UAGX_UpsamplingParticleRendererComponent::SetUpsampling(int32 InUpsampling)
{
	return Upsampling = InUpsampling;
}

int32 UAGX_UpsamplingParticleRendererComponent::GetUpsampling()
{
	return Upsampling;
}

bool UAGX_UpsamplingParticleRendererComponent::SetEnableVoxelSize(bool bEnabled)
{
	return bEnableVoxelSize = bEnabled;
}

bool UAGX_UpsamplingParticleRendererComponent::GetEnableVoxelSize()
{
	return bEnableVoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::SetVoxelSize(double InVoxelSize)
{
	return VoxelSize = InVoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetVoxelSize()
{
	return VoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::SetEaseStepSize(double InEaseStepSize)
{
	return EaseStepSize = InEaseStepSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetEaseStepSize()
{
	return EaseStepSize;
}

/**
 *
 */
UNiagaraSystem* UAGX_UpsamplingParticleRendererComponent::FindNiagaraSystemAsset(const TCHAR* AssetPath)
{
	auto AssetFinder = ConstructorHelpers::FObjectFinder<UNiagaraSystem>(AssetPath);
	if (!AssetFinder.Succeeded())
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), AssetPath);
		return nullptr;
	}

	return AssetFinder.Object;
}

/**
 *
 */
bool UAGX_UpsamplingParticleRendererComponent::InitializeParentTerrainActor()
{
	// First get parent actor
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle Renderer '%s' could not fetch the parent actor"
				 "particles"),
			*GetName());
		return false;
	}

	// Then cast it to the proper type
	ParentTerrainActor = Cast<AAGX_Terrain>(Owner);
	if (!ParentTerrainActor)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle Renderer '%s' could not cast parent to 'AGX_Terrain' actor, cannot "
				 "fetch particle data"
				 "particles"),
			*GetName());
		return false;
	}

	return ParentTerrainActor != nullptr;
}

/**
 *
 */
bool UAGX_UpsamplingParticleRendererComponent::InitializeNiagaraParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle renderer '%s' does not have a particle system, cannot render particles "
				 "with Niagara"),
			*GetName());
		return false;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, ParentTerrainActor->GetRootComponent(), NAME_None, FVector::ZeroVector,
		FRotator::ZeroRotator, FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
#if WITH_EDITORONLY_DATA
	// Must check for nullptr here because no particle system component is created with running
	// as a unit test without graphics, i.e. with our run_unit_tests script in GitLab CI.
	if (ParticleSystemComponent != nullptr)
	{
		ParticleSystemComponent->bVisualizeComponent = true;
	}
#endif

	return ParticleSystemComponent != nullptr;
}

/**
 *
 */
void UAGX_UpsamplingParticleRendererComponent::HandleParticleData(FParticleDataById data)
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

	const int32 NumParticles = Positions.Num();
	TArray<FVector4> PositionsAndScale;
	PositionsAndScale.SetNum(NumParticles);
	TArray<FVector4> Orientations;
	Orientations.SetNum(NumParticles);
	//TArray<FCoarseParticle> NewCoarseParticles;
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
		if (Exists[I])
		{
			//float Mass = Masses[I]; // Convert to gram
			float Volume = (4.0 / 3.0) * PI * FMath::Pow(Radii[I], 3);
			//ParticleDensity = Mass / Volume;
			AppendIfActiveVoxel(ActiveVoxelSet, Positions[I], Radii[I]);
			//FCoarseParticle CP;
			//CP.PositionAndRadius =
			//	FVector4f(Positions[I].X, Positions[I].Y, Positions[I].Z, Radii[I]);
			//CP.VelocityAndMass =
			//	FVector4f(Velocities[I].X, Velocities[I].Y, Velocities[I].Z, Masses[I]);
			//NewCoarseParticles.Add(CP);
		}
	}
	//if (ParticleDensity == 0.0f)
	//{
	//	return;
	//}

	const float ElementSize = 15.0f;
	TArray<FIntVector4> ActiveVoxelIndices = GetActiveVoxelsFromSet(ActiveVoxelSet);

	// UParticleUpsamplingInterface::SetCoarseParticles(NewCoarseParticles);
	// UParticleUpsamplingInterface::SetActiveVoxelIndices(ActiveVoxelIndices);
	// UParticleUpsamplingInterface::RecalculateFineParticleProperties(Upsampling, ElementSize, ParticleDensity);
	// UParticleUpsamplingInterface::SetStaticVariables(VoxelSize, EaseStepSize);
	// int HashTableSize = UParticleUpsamplingInterface::GetHashTableSize();
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

/**
 *
 */
void UAGX_UpsamplingParticleRendererComponent::AppendIfActiveVoxel(
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

/**
 *
 */
TArray<FIntVector4> UAGX_UpsamplingParticleRendererComponent::GetActiveVoxelsFromSet(
	TSet<FIntVector> VoxelSet)
{
	TArray<FIntVector4> ActiveVoxels;
	for (FIntVector Voxel : VoxelSet)
	{
		ActiveVoxels.Add(FIntVector4(Voxel, 0));
	}
	return ActiveVoxels;
}

#if WITH_EDITOR

/**
 *
 */
bool UAGX_UpsamplingParticleRendererComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	const FName Prop = InProperty->GetFName();

	if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_UpsamplingParticleRendererComponent, ParticleSystemAsset))
	{
		return false;
	}

	return SuperCanEditChange;
}

/**
 *
 */
void UAGX_UpsamplingParticleRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

/**
 *
 */
void UAGX_UpsamplingParticleRendererComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

/**
 *
 */
void UAGX_UpsamplingParticleRendererComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bEnableParticleRendering),
		[](ThisClass* This) { This->SetEnableParticleRendering(This->bEnableParticleRendering); });
}

#endif
