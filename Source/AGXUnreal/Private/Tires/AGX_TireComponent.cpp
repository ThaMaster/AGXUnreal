#include "Tires/AGX_TireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

UAGX_TireComponent::UAGX_TireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_TireComponent::HasNative() const
{
	return NativeBarrier.Get() != nullptr && NativeBarrier->HasNative();
}

FTireBarrier* UAGX_TireComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

FTireBarrier* UAGX_TireComponent::GetNative()
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

const FTireBarrier* UAGX_TireComponent::GetNative() const
{
	if (NativeBarrier)
	{
		return NativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

void UAGX_TireComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		CreateNative();
	}
}

void UAGX_TireComponent::CreateNative()
{
	check(!HasNative());

	AllocateNative();

	if (HasNative())
	{
		UpdateNativeProperties();
		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
		Simulation->GetNative()->AddTire(NativeBarrier.Get());
	}
	else
	{
		UE_LOG(
			LogAGX, Error, TEXT("Tire %s in %s: Unable to create Tire."),
			*GetFName().ToString(), *GetOwner()->GetName());
	}
}
