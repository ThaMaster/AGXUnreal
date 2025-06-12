// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_BaseTerrainParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

UAGX_BaseTerrainParticleRendererComponent::UAGX_BaseTerrainParticleRendererComponent()
{
	// Constructor
}

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

	// Maybe remove this to allow user to bind to whatever they want?
	BindParticleHandler();
}

void UAGX_BaseTerrainParticleRendererComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnbindParticleHandler();
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

// TODO: Make it possible to choose which delegate and function to bind to!
void UAGX_BaseTerrainParticleRendererComponent::BindParticleHandler()
{
	DelegateHandle = ParentTerrainActor->UpdateParticleDataDelegate.AddLambda(
		[this](FParticleDataById data) { HandleParticleData(data); }
	);
}

void UAGX_BaseTerrainParticleRendererComponent::UnbindParticleHandler()
{
	if (DelegateHandle.IsValid())
	{
		ParentTerrainActor->UpdateParticleDataDelegate.Remove(DelegateHandle);
	}
}

void UAGX_BaseTerrainParticleRendererComponent::HandleParticleData(FParticleDataById data)
{
	return;
}

#if WITH_EDITOR

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
		AGX_MEMBER_NAME(bEnableParticleRendering),
		[](ThisClass* This) { This->SetEnableParticleRendering(This->bEnableParticleRendering); });
}

#endif
