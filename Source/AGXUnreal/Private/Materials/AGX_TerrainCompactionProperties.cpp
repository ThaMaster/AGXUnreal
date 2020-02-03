#include "Materials/AGX_TerrainCompactionProperties.h"

FAGX_TerrainCompactionProperties::FAGX_TerrainCompactionProperties()
	: AngleOfReposeCompactionRate(1.0)
	, Phi0(1.0 / (1.0 + 0.5))
	, CompactionTimeRelaxationConstant(0.05)
	, CompressionIndex(0.1)
	, K_e(1.0)
	, N_e(0.5)
	, PreconsolidationStress(98e3)
	, StressCutOffFraction(0.01)
{
	// See agx\src\agxTerrain\TerrainMaterial.cpp for default values
}
