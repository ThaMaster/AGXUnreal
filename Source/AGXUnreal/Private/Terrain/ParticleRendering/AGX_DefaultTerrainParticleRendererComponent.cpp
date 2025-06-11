// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_DefaultTerrainParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

UAGX_DefaultTerrainParticleRendererComponent::UAGX_DefaultTerrainParticleRendererComponent()
{
	const TCHAR* Path = TEXT(
		"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
		"PS_SoilParticleSystem.PS_SoilParticleSystem'");

	if (ParticleSystemAsset != nullptr)
		return;

	using Type = typename std::remove_reference<decltype(*ParticleSystemAsset)>::type;
	auto AssetFinder = ConstructorHelpers::FObjectFinder<Type>(Path);
	if (!AssetFinder.Succeeded())
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), Path);
		return;
	}

	ParticleSystemAsset = AssetFinder.Object;

}

UNiagaraComponent* UAGX_DefaultTerrainParticleRendererComponent::GetSpawnedParticleSystemComponent()
{
	return ParticleSystemComponent;
}

void UAGX_DefaultTerrainParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableParticleRendering) // Maybe bad?
	{
		if(InitializeParticleSystem())
		{
			return;
		}
	}

}

void UAGX_DefaultTerrainParticleRendererComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


bool UAGX_DefaultTerrainParticleRendererComponent::InitializeParticleSystem()
{	
	return InitializeParticleSystemComponent();
}

bool UAGX_DefaultTerrainParticleRendererComponent::InitializeParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle Renderer '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return false;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	/* ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, TerrainActor->GetRootComponent(), NAME_None, FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
	*/
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

void UAGX_DefaultTerrainParticleRendererComponent::UpdateParticleData(FParticleDataById& data)
{
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
