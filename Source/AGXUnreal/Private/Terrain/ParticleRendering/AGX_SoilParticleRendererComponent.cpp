// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_SoilParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/AGX_ParticleRenderingUtilities.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArray.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"

const FName UAGX_SoilParticleRendererComponent::PositionsAndScalesName(
	TEXT("User.Positions And Scales"));
const FName UAGX_SoilParticleRendererComponent::VelocitiesAndMassesName(
	TEXT("User.Velocities And Masses"));
const FName UAGX_SoilParticleRendererComponent::OrientationsName(
	TEXT("User.Orientations"));
const FName UAGX_SoilParticleRendererComponent::ExistsName(
	TEXT("User.Exists"));
const FName UAGX_SoilParticleRendererComponent::ParticleCountName(
	TEXT("User.Particle Count"));

const FName UAGX_SoilParticleRendererComponent::Vector4ArrayName(
	TEXT("NiagaraDataInterfaceArrayFloat4"));
const FName UAGX_SoilParticleRendererComponent::BoolArrayName(
	TEXT("NiagaraDataInterfaceArrayBool"));
const FName UAGX_SoilParticleRendererComponent::Int32Name(
	TEXT("NiagaraInt32"));


UAGX_SoilParticleRendererComponent::UAGX_SoilParticleRendererComponent()
{
	AGX_ParticleRenderingUtilities::AssignDefaultNiagaraAsset(
		ParticleSystemAsset,
		TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/SoilParticleSystem/"
			 "PS_SoilParticleSystem.PS_SoilParticleSystem'"));
}

void UAGX_SoilParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!bEnableParticleRendering)
	{
		return;
	}

	AAGX_Terrain* ParentTerrainActor = AGX_ParticleRenderingUtilities::GetParentTerrainActor(this);
	if (!ParentTerrainActor)
	{
		return;
	}

	ParticleSystemComponent =
		AGX_ParticleRenderingUtilities::InitializeNiagaraParticleSystemComponent(
			ParticleSystemAsset, this);
	if (!ParticleSystemComponent)
	{
		return;
	}

	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->OnParticleData.AddDynamic(
		this, &UAGX_SoilParticleRendererComponent::HandleParticleData);
}

bool UAGX_SoilParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateImmediate();
		ParticleSystemComponent->SetActive(bEnabled);
	}
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

void UAGX_SoilParticleRendererComponent::HandleParticleData(FDelegateParticleData& data)
{
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	const FNiagaraParameterStore& UserParams = ParticleSystemComponent->GetOverrideParameters();
	TArray<FNiagaraVariable> Params;
	UserParams.GetParameters(Params);

	// Check for the default particle system parameters.
	// Otherwise the engine throws warnings each time we set them...
	for (FNiagaraVariable& Param : Params)
	{
		FName ParamName = Param.GetName();
		FName ParamType = Param.GetType().GetFName();

		if (ParamType == Vector4ArrayName)
		{
			if (ParamName == PositionsAndScalesName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, PositionsAndScalesName, data.PositionsAndScale);
			}
			else if (ParamName == OrientationsName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, OrientationsName, data.Orientations);
			}
			else if (ParamName == VelocitiesAndMassesName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, VelocitiesAndMassesName, data.VelocitiesAndMasses);
			}
		}
		else if (ParamName == ExistsName && ParamType == BoolArrayName)
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
				ParticleSystemComponent, ExistsName, data.Exists);
		}
		else if (ParamName == ParticleCountName && ParamType == Int32Name)
		{

#if UE_VERSION_OLDER_THAN(5, 3, 0)
			ParticleSystemComponent->SetNiagaraVariableInt(ParticleCountName, data.ParticleCount);
#else
			ParticleSystemComponent->SetVariableInt(FName(ParticleCountName), data.ParticleCount);
#endif
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
