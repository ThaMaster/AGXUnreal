#include "Materials/AGX_ShapeMaterialInstance.h"

#include "Engine/World.h"

#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/MaterialBarrier.h"

UAGX_ShapeMaterialInstance* UAGX_ShapeMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_MaterialBase* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_Instance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_ShapeMaterialInstance::CreateFromAsset is creating an instance named %s "
			 "(from asset %s)."),
		*InstanceName, *Source->GetName());

	UAGX_ShapeMaterialInstance* NewInstance = NewObject<UAGX_ShapeMaterialInstance>(
		Outer, UAGX_ShapeMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyProperties(Source);

	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

UAGX_ShapeMaterialInstance::~UAGX_ShapeMaterialInstance()
{
}

FMaterialBarrier* UAGX_ShapeMaterialInstance::GetOrCreateNative(UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

FMaterialBarrier* UAGX_ShapeMaterialInstance::GetNative()
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

bool UAGX_ShapeMaterialInstance::HasNative() const
{
	return NativeBarrier && NativeBarrier->HasNative();
}

void UAGX_ShapeMaterialInstance::UpdateNativeProperties()
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

UAGX_ShapeMaterialInstance* UAGX_ShapeMaterialInstance::GetOrCreateShapeMaterialInstance(
	UWorld* PlayingWorld)
{
	return this;
};

void UAGX_ShapeMaterialInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FMaterialBarrier());

	NativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Simulation);

	Simulation->GetNative()->AddMaterial(NativeBarrier.Get());
}
