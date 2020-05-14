#include "Materials/AGX_MaterialBase.h"

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
