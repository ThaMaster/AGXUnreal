#include "Materials/AGX_TerrainMaterialBase.h"

#include "Materials/AGX_TerrainMaterialInstance.h"
#include "AGX_LogCategory.h"

#include "Engine/World.h"

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialBase::GetOrCreateTerrainMaterialInstance(
	UWorld* PlayingWorld, UAGX_TerrainMaterialBase* Property)
{
	if (Property == nullptr || PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		return nullptr;
	}

	UAGX_TerrainMaterialInstance* Instance =
		Property->GetOrCreateTerrainMaterialInstance(PlayingWorld);

	if (Instance != Property)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("UAGX_TerrainMaterialBase::GetOrCreateTerrainMaterialInstance is swapping a "
				 " property (to %s from %s)."),
			*GetNameSafe(Instance), *GetNameSafe(Property));

		Property = Instance;
	}

	return Instance;
}
