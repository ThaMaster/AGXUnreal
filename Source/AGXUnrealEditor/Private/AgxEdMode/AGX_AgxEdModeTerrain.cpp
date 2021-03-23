#include "AgxEdMode/AGX_AgxEdModeTerrain.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeTerrain"

UAGX_AgxEdModeTerrain* UAGX_AgxEdModeTerrain::GetInstance()
{
	static UAGX_AgxEdModeTerrain* TerrainSubmode = nullptr;
	if (TerrainSubmode == nullptr)
	{
		TerrainSubmode = GetMutableDefault<UAGX_AgxEdModeTerrain>();
	}
	return TerrainSubmode;
}

FText UAGX_AgxEdModeTerrain::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Terrain");
}

FText UAGX_AgxEdModeTerrain::GetTooltip() const
{
	return LOCTEXT(
		"Tooltip", "Management of AGX Terrain assets.");
}

#undef LOCTEXT_NAMESPACE
