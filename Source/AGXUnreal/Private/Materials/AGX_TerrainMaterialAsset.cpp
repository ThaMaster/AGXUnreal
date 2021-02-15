#include "Materials/AGX_TerrainMaterialAsset.h"

#include "Engine/World.h"

#include "Materials/AGX_TerrainMaterialInstance.h"

UAGX_MaterialBase* UAGX_TerrainMaterialAsset::GetOrCreateInstance(UWorld* PlayingWorld)
{
	UAGX_TerrainMaterialInstance* InstancePtr = TerrainMaterialInstance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TerrainMaterialInstance::CreateFromAsset(PlayingWorld, this);
		TerrainMaterialInstance = InstancePtr;
	}

	return InstancePtr;
}
