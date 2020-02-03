#include "Materials/AGX_TerrainMaterialAsset.h"

#include "Engine/World.h"

#include "Materials/AGX_TerrainMaterialInstance.h"
#include "Materials/AGX_ShapeMaterialInstance.h"

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialAsset::GetOrCreateTerrainMaterialInstance(
	UWorld* PlayingWorld)
{
	UAGX_TerrainMaterialInstance* InstancePtr = TerrainMaterialInstance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_TerrainMaterialInstance::CreateFromAsset(PlayingWorld, this);
		TerrainMaterialInstance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_ShapeMaterialInstance* UAGX_TerrainMaterialAsset::GetOrCreateShapeMaterialInstance(
	UWorld* PlayingWorld)
{
	UAGX_ShapeMaterialInstance* InstancePtr = MaterialInstance.Get();

	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr =
			UAGX_ShapeMaterialInstance::CreateFromAsset(PlayingWorld, this);
		MaterialInstance = InstancePtr;
	}

	return InstancePtr;
}
