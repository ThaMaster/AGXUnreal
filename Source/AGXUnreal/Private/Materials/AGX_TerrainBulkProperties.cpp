// Copyright 2022, Algoryx Simulation AB.


#include "Materials/AGX_TerrainBulkProperties.h"

FAGX_TerrainBulkProperties::FAGX_TerrainBulkProperties()
	: AdhesionOverlapFactor(0.05)
	, Cohesion(0.0)
	, Density(1400.0)
	, DilatancyAngle(15)
	, FrictionAngle(45)
	, MaxDensity(1600)
	, PoissonsRatio(0.1)
	, SwellFactor(1.1)
	, YoungsModulus(1.0e7)
{
}
