#include "Materials/AGX_MaterialInstance.h"

#include "AGX_Simulation.h"

#include "Engine/World.h"

#include "AGX_LogCategory.h"
#include "Materials/AGX_MaterialAsset.h"
#include "AGX_Simulation.h"
#include "Materials/MaterialBarrier.h"

UAGX_MaterialInstance* UAGX_MaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_MaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_MaterialBase::CreateFromAsset is creating an instance named \"%s\" (from asset "
			 "\"%s\")."),
		*InstanceName, *Source->GetName());

	UAGX_MaterialInstance* NewInstance = NewObject<UAGX_MaterialInstance>(
		Outer, UAGX_MaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyProperties(Source);
	NewInstance->SourceAsset = Source;

	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

UAGX_MaterialInstance::~UAGX_MaterialInstance()
{
}

UAGX_MaterialAsset* UAGX_MaterialInstance::GetAsset()
{
	return SourceAsset.Get();
}

FMaterialBarrier* UAGX_MaterialInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

FMaterialBarrier* UAGX_MaterialInstance::GetNative()
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

bool UAGX_MaterialInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_MaterialInstance::UpdateNativeProperties()
{
	if (HasNative())
	{
		NativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		NativeBarrier->SetDensity(Bulk.Density);
		NativeBarrier->SetYoungsModulus(Bulk.YoungsModulus);
		NativeBarrier->SetBulkViscosity(Bulk.Viscosity);
		NativeBarrier->SetDamping(Bulk.Damping);
		NativeBarrier->SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		NativeBarrier->SetFrictionEnabled(Surface.bFrictionEnabled);
		NativeBarrier->SetRoughness(Surface.Roughness);
		NativeBarrier->SetSurfaceViscosity(Surface.Viscosity);
		NativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
	}
}

UAGX_MaterialInstance* UAGX_MaterialInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
};

void UAGX_MaterialInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FMaterialBarrier());

	NativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Simulation);

	Simulation->GetNative()->AddMaterial(NativeBarrier.Get());
}
