#include "Materials/AGX_TerrainMaterialBase.h"

#include "Engine/World.h"

void UAGX_TerrainMaterialBase::CopyTerrainMaterialProperties(const UAGX_TerrainMaterialBase* Source)
{
	if (Source)
	{
		// As of now, this property is not used for terrain (replaced by the terrain specific bulk
		// properties) and will always have default values.
		Bulk = Source->Bulk;

		Surface = Source->Surface;
		TerrainBulk = Source->TerrainBulk;
		TerrainCompaction = Source->TerrainCompaction;
	}
}
