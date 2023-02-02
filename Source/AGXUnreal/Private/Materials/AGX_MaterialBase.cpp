// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_MaterialBase.h"

// Unreal Engine includes.
#include "Engine/World.h"


void UAGX_MaterialBase::CopyShapeMaterialProperties(const UAGX_MaterialBase* Source)
{
	if (Source)
	{
		Bulk = Source->Bulk;
		Surface = Source->Surface;
		Wire = Source->Wire;
	}
}
