// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_BaseTerrainParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

void UAGX_BaseTerrainParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bEnableParticleRendering)
	{
		return;
	}

	if (!ParentTerrainActor)
	{
		// Fetch the parent terrain actor which holds the particle data
		if (!InitializeParentTerrainActor())
		{
			return;
		}
	}
}

void UAGX_BaseTerrainParticleRendererComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// If delegate is has not been removed, remoe it
	if (DelegateHandle.IsValid())
	{
		//ParentTerrainActor->UpdateParticleDataDelegate.Remove(DelegateHandle);
	}
}

UNiagaraSystem* UAGX_BaseTerrainParticleRendererComponent::FindNiagaraSystemAsset(const TCHAR* AssetPath)
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

void UAGX_BaseTerrainParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	bEnableParticleRendering = bEnabled;
}

UNiagaraComponent* UAGX_BaseTerrainParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

bool UAGX_BaseTerrainParticleRendererComponent::InitializeNiagaraParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle renderer '%s' does not have a particle system, cannot render particles with Niagara"),
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

bool UAGX_BaseTerrainParticleRendererComponent::InitializeParentTerrainActor()
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

void UAGX_BaseTerrainParticleRendererComponent::HandleParticleData(FParticleDataById data)
{
	return;
}

#if WITH_EDITOR

bool UAGX_BaseTerrainParticleRendererComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	const FName Prop = InProperty->GetFName();

	if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_BaseTerrainParticleRendererComponent, ParticleSystemAsset))
	{
		return false;
	}

	return SuperCanEditChange;
}

void UAGX_BaseTerrainParticleRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_BaseTerrainParticleRendererComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_BaseTerrainParticleRendererComponent::InitPropertyDispatcher()
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
