// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_SoilParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArray.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraEmitterInstance.h"

UAGX_SoilParticleRendererComponent::UAGX_SoilParticleRendererComponent()
{
	AssignDefaultNiagaraAsset(
		ParticleSystemAsset,
		TEXT(
			"NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/SoilParticleSystem/"
			"PS_SoilParticleSystem.PS_SoilParticleSystem'"
		)
	);
}

void UAGX_SoilParticleRendererComponent::BeginPlay()
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
	ParentTerrainActor->UpdateParticleDataDelegate.AddDynamic(this, &UAGX_SoilParticleRendererComponent::HandleParticleData);
}

void UAGX_SoilParticleRendererComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

bool UAGX_SoilParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	return bEnableParticleRendering = bEnabled;
}

bool UAGX_SoilParticleRendererComponent::GetEnableParticleRendering()
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_SoilParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

int32 UAGX_SoilParticleRendererComponent::GetNumParticles()
{
	int32 TotalNumParticles = 0;
	FNiagaraSystemInstance* SystemInstance = ParticleSystemComponent->GetSystemInstance(); // THIS IS DEPRECATED!
	if (SystemInstance)
	{
		for (const TSharedRef<FNiagaraEmitterInstance>& EmitterInstance :
			 SystemInstance->GetEmitters())
		{
			TotalNumParticles += EmitterInstance->GetNumParticles();
		}
	}

	return TotalNumParticles;
}

void UAGX_SoilParticleRendererComponent::AssignDefaultNiagaraAsset(
	auto*& AssetRefProperty, const TCHAR* AssetPath)
{
	if (AssetRefProperty != nullptr)
		return;

	using Type = typename std::remove_reference<decltype(*AssetRefProperty)>::type;
	auto AssetFinder = ConstructorHelpers::FObjectFinder<Type>(AssetPath);
	if (!AssetFinder.Succeeded())
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), AssetPath);
		return;
	}

	AssetRefProperty = AssetFinder.Object;
}

bool UAGX_SoilParticleRendererComponent::InitializeParentTerrainActor()
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

bool UAGX_SoilParticleRendererComponent::InitializeNiagaraParticleSystemComponent()
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

void UAGX_SoilParticleRendererComponent::HandleParticleData(FDelegateParticleData data)
{
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	// TODO: Also check that this exists!
#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt("User.Target Particle Count", data.TargetCount);
#else
	ParticleSystemComponent->SetVariableInt(FName("User.Target Particle Count"), data.TargetCount);
#endif

	const FNiagaraParameterStore& UserParams = ParticleSystemComponent->GetOverrideParameters();
	TArray<FNiagaraVariable> Params;
	UserParams.GetParameters(Params);

	// TODO: Make this for-loop better
	for (FNiagaraVariable& Param : Params)
	{
		FString ParamName = Param.GetName().ToString();
		FString ParamType = Param.GetType().GetName();

		if (ParamType == "NiagaraDataInterfaceArrayFloat4")
		{
			if (ParamName == "User.Positions And Scales")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Positions And Scales", data.PositionsAndScale);
			}
			else if (ParamName == "User.Orientations")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Orientations", data.Orientations);
			}
			else if (ParamName == "User.Velocities And Masses")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Velocities And Masses", data.VelocitiesAndMasses);
			}
		}
		else if (
			ParamName == "User.Exists" && 
			ParamType == "NiagaraDataInterfaceArrayBool")
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
				ParticleSystemComponent, "Exists", data.Exists);
		}
	}
}

#if WITH_EDITOR

void UAGX_SoilParticleRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_SoilParticleRendererComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_SoilParticleRendererComponent::InitPropertyDispatcher()
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

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

}

#endif
