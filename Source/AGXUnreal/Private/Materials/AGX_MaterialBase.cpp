#include "Materials/AGX_MaterialBase.h"

// AGXUnreal includes.
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

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

void UAGX_MaterialBase::CopyShapeMaterialProperties(const FShapeMaterialBarrier* Source)
{
	if (Source)
	{
		// Copy shape material bulk properties.
		Bulk = FAGX_ShapeMaterialBulkProperties();
		Bulk.Density = Source->GetDensity();
		Bulk.YoungsModulus = Source->GetYoungsModulus();
		Bulk.Viscosity = Source->GetBulkViscosity();
		Bulk.Damping = Source->GetDamping();
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
