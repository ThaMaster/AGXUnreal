#include "Materials/AGX_ShapeMaterialInstance.h"

#include "Engine/World.h"

#include "AGX_Simulation.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/AGX_ShapeMaterialAsset.h"


void UAGX_ShapeMaterialInstance::SetDensity(float InDensity)
{
	if (HasNative())
	{
		NativeBarrier->SetDensity(static_cast<double>(InDensity));
	}

	Bulk.Density = InDensity;
}

float UAGX_ShapeMaterialInstance::GetDensity() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.Density;
}

void UAGX_ShapeMaterialInstance::SetYoungsModulus(float InYoungsModulus)
{
	if (HasNative())
	{
		NativeBarrier->SetYoungsModulus(static_cast<double>(InYoungsModulus));
	}

	Bulk.YoungsModulus = InYoungsModulus;
}

float UAGX_ShapeMaterialInstance::GetYoungsModulus() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.YoungsModulus;
}

void UAGX_ShapeMaterialInstance::SetBulkViscosity(float InBulkViscosity)
{
	if (HasNative())
	{
		NativeBarrier->SetBulkViscosity(static_cast<double>(InBulkViscosity));
	}

	Bulk.Viscosity = InBulkViscosity;
}

float UAGX_ShapeMaterialInstance::GetBulkViscosity() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.Viscosity;
}

void UAGX_ShapeMaterialInstance::SetDamping(float InDamping)
{
	if (HasNative())
	{
		NativeBarrier->SetDamping(static_cast<double>(InDamping));
	}

	Bulk.Damping = InDamping;
}

float UAGX_ShapeMaterialInstance::GetDamping() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.Damping;
}

void UAGX_ShapeMaterialInstance::SetMinMaxElasticRestLength(float InMin, float InMax)
{
	if (HasNative())
	{
		NativeBarrier->SetMinMaxElasticRestLength(static_cast<double>(InMin), static_cast<double>(InMax));
	}

	Bulk.MinElasticRestLength = InMin;
	Bulk.MaxElasticRestLength = InMax;
}

float UAGX_ShapeMaterialInstance::GetMinElasticRestLength() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.MinElasticRestLength;
}

float UAGX_ShapeMaterialInstance::GetMaxElasticRestLength() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetDamping());
	}

	return Bulk.MaxElasticRestLength;
}

void UAGX_ShapeMaterialInstance::SetFrictionEnabled(bool Enabled)
{
	if (HasNative())
	{
		NativeBarrier->SetFrictionEnabled(Enabled);
	}

	Surface.bFrictionEnabled = Enabled;
}

bool UAGX_ShapeMaterialInstance::GetFrictionEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_ShapeMaterialInstance::SetRoughness(float Roughness)
{
	if (HasNative())
	{
		NativeBarrier->SetRoughness(static_cast<double>(Roughness));
	}

	Surface.bFrictionEnabled = static_cast<double>(Roughness);
}

float UAGX_ShapeMaterialInstance::GetRoughness() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetRoughness());
	}

	return static_cast<float>(Surface.Roughness);
}

void UAGX_ShapeMaterialInstance::SetSurfaceViscosity(float Viscosity)
{
	if (HasNative())
	{
		NativeBarrier->SetSurfaceViscosity(static_cast<double>(Viscosity));
	}

	Surface.Viscosity = static_cast<double>(Viscosity);
}

float UAGX_ShapeMaterialInstance::GetSurfaceViscosity() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetSurfaceViscosity());
	}

	return static_cast<float>(Surface.Viscosity);
}

void UAGX_ShapeMaterialInstance::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	if (HasNative())
	{
		NativeBarrier->SetAdhesion(static_cast<double>(AdhesiveForce), static_cast<double>(AdhesiveOverlap));
	}

	Surface.AdhesiveForce = static_cast<double>(AdhesiveForce);
	Surface.AdhesiveOverlap = static_cast<double>(AdhesiveOverlap);
}

float UAGX_ShapeMaterialInstance::GetAdhesiveForce() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetAdhesiveForce());
	}

	return static_cast<float>(Surface.AdhesiveForce);
}

float UAGX_ShapeMaterialInstance::GetAdhesiveOverlap() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier->GetAdhesiveOverlap());
	}

	return static_cast<float>(Surface.AdhesiveOverlap);
}

UAGX_ShapeMaterialInstance* UAGX_ShapeMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_ShapeMaterialInstance";

	UAGX_ShapeMaterialInstance* NewInstance = NewObject<UAGX_ShapeMaterialInstance>(
		Outer, UAGX_ShapeMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	NewInstance->CopyShapeMaterialProperties(Source);

	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

UAGX_ShapeMaterialInstance::~UAGX_ShapeMaterialInstance()
{
}

FShapeMaterialBarrier* UAGX_ShapeMaterialInstance::GetOrCreateShapeMaterialNative(
	UWorld* PlayingWorld)
{
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

FShapeMaterialBarrier* UAGX_ShapeMaterialInstance::GetNative()
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

UAGX_MaterialBase* UAGX_ShapeMaterialInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
};

void UAGX_ShapeMaterialInstance::CreateNative(UWorld* PlayingWorld)
{
	NativeBarrier.Reset(new FShapeMaterialBarrier());

	NativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Simulation);

	Simulation->GetNative()->AddShapeMaterial(NativeBarrier.Get());
}
