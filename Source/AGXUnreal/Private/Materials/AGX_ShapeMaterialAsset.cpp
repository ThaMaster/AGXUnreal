#include "Materials/AGX_ShapeMaterialAsset.h"

#include "Engine/World.h"

#include "Materials/AGX_ShapeMaterialInstance.h"

UAGX_ShapeMaterialInstance* UAGX_ShapeMaterialAsset::GetOrCreateShapeMaterialInstance(UWorld* PlayingWorld)
{
	UAGX_ShapeMaterialInstance* InstancePtr = Instance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeMaterialInstance::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
};
