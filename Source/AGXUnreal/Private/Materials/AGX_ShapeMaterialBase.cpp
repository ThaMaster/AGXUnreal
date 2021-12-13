// Copyright 2021, Algoryx Simulation AB.


#include "Materials/AGX_ShapeMaterialBase.h"

// AGX Dynamics for Unreal includes.
#include "Materials/ShapeMaterialBarrier.h"

void UAGX_ShapeMaterialBase::SetDensity(float InDensity)
{
	Bulk.Density = static_cast<double>(InDensity);
}

float UAGX_ShapeMaterialBase::GetDensity() const
{
	return static_cast<float>(Bulk.Density);
}

void UAGX_ShapeMaterialBase::SetYoungsModulus(float InYoungsModulus)
{
	Bulk.YoungsModulus = static_cast<double>(InYoungsModulus);
}

float UAGX_ShapeMaterialBase::GetYoungsModulus() const
{
	return static_cast<float>(Bulk.YoungsModulus);
}

void UAGX_ShapeMaterialBase::SetBulkViscosity(float InBulkViscosity)
{
	Bulk.Viscosity = static_cast<double>(InBulkViscosity);
}

float UAGX_ShapeMaterialBase::GetBulkViscosity() const
{
	return static_cast<float>(Bulk.Viscosity);
}

void UAGX_ShapeMaterialBase::SetSpookDamping(float InSpookDamping)
{
	Bulk.SpookDamping = static_cast<double>(InSpookDamping);
}

float UAGX_ShapeMaterialBase::GetSpookDamping() const
{
	return static_cast<float>(Bulk.SpookDamping);
}

void UAGX_ShapeMaterialBase::SetMinMaxElasticRestLength(float InMin, float InMax)
{
	Bulk.MinElasticRestLength = static_cast<double>(InMin);
	Bulk.MaxElasticRestLength = static_cast<double>(InMax);
}

float UAGX_ShapeMaterialBase::GetMinElasticRestLength() const
{
	return static_cast<float>(Bulk.MinElasticRestLength);
}

float UAGX_ShapeMaterialBase::GetMaxElasticRestLength() const
{
	return static_cast<float>(Bulk.MaxElasticRestLength);
}

float UAGX_ShapeMaterialBase::GetYoungsModulusStretch() const
{
	return Wire.YoungsModulusStretch;
}

void UAGX_ShapeMaterialBase::SetYoungsModulusStretch(float InYoungsModulus)
{
	Wire.YoungsModulusStretch = InYoungsModulus;
}

float UAGX_ShapeMaterialBase::GetYoungsModulusBend() const
{
	return Wire.YoungsModulusBend;
}

void UAGX_ShapeMaterialBase::SetYoungsModulusBend(float InYoungsModulus)
{
	Wire.YoungsModulusBend = InYoungsModulus;
}

float UAGX_ShapeMaterialBase::GetSpookDampingStretch() const
{
	return Wire.SpookDampingStretch;
}

void UAGX_ShapeMaterialBase::SetSpookDampingStretch(float InSpookDamping)
{
	Wire.SpookDampingStretch = InSpookDamping;
}

float UAGX_ShapeMaterialBase::GetSpookDampingBend() const
{
	return Wire.SpookDampingBend;
}

void UAGX_ShapeMaterialBase::SetSpookDampingBend(float InSpookDamping)
{
	Wire.SpookDampingStretch = InSpookDamping;
}

void UAGX_ShapeMaterialBase::CopyFrom(const FShapeMaterialBarrier* Source)
{
	if (Source)
	{
		// Copy shape material bulk properties.
		Bulk = FAGX_ShapeMaterialBulkProperties();
		Bulk.Density = Source->GetDensity();
		Bulk.YoungsModulus = Source->GetYoungsModulus();
		Bulk.Viscosity = Source->GetBulkViscosity();
		Bulk.SpookDamping = Source->GetSpookDamping();
		Bulk.MinElasticRestLength = Source->GetMinElasticRestLength();
		Bulk.MaxElasticRestLength = Source->GetMaxElasticRestLength();

		// Copy shape material surface properties.
		Surface = FAGX_ShapeMaterialSurfaceProperties();
		Surface.bFrictionEnabled = Source->GetFrictionEnabled();
		Surface.Roughness = Source->GetRoughness();
		Surface.Viscosity = Source->GetSurfaceViscosity();
		Surface.AdhesiveForce = Source->GetAdhesiveForce();
		Surface.AdhesiveOverlap = Source->GetAdhesiveOverlap();
	}
}
