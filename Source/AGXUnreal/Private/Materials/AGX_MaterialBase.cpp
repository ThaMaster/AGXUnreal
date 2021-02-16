#include "Materials/AGX_MaterialBase.h"

// Unreal Engine includes.
#include "Engine/World.h"

void UAGX_MaterialBase::SetFrictionEnabled(bool Enabled)
{
	Surface.bFrictionEnabled = Enabled;
}

bool UAGX_MaterialBase::GetFrictionEnabled() const
{
	return Surface.bFrictionEnabled;
}

void UAGX_MaterialBase::SetRoughness(float Roughness)
{
	Surface.Roughness = static_cast<double>(Roughness);
}

float UAGX_MaterialBase::GetRoughness() const
{
	return static_cast<float>(Surface.Roughness);
}

void UAGX_MaterialBase::SetSurfaceViscosity(float Viscosity)
{
	Surface.Viscosity = static_cast<double>(Viscosity);
}

float UAGX_MaterialBase::GetSurfaceViscosity() const
{
	return static_cast<float>(Surface.Viscosity);
}

void UAGX_MaterialBase::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	Surface.AdhesiveForce = static_cast<double>(AdhesiveForce);
	Surface.AdhesiveOverlap = static_cast<double>(AdhesiveOverlap);
}

float UAGX_MaterialBase::GetAdhesiveForce() const
{
	return static_cast<float>(Surface.AdhesiveForce);
}

float UAGX_MaterialBase::GetAdhesiveOverlap() const
{
	return static_cast<float>(Surface.AdhesiveOverlap);
}

UAGX_MaterialBase::~UAGX_MaterialBase()
{
}

void UAGX_MaterialBase::CopyShapeMaterialProperties(const UAGX_MaterialBase* Source)
{
	if (Source)
	{
		Bulk = Source->Bulk;
		Surface = Source->Surface;
	}
}
