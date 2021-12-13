// Copyright 2021, Algoryx Simulation AB.


#include "Materials/AGX_ContactMaterialReductionMode.h"

FAGX_ContactMaterialReductionMode::FAGX_ContactMaterialReductionMode()
	: Mode(EAGX_ContactReductionMode::Geometry)
	, BinResolution(0)
{
	// See agx\src\agx\Material.cpp for default values
}
