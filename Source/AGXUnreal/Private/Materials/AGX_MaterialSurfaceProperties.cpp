// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_MaterialSurfaceProperties.h"


FAGX_MaterialSurfaceProperties::FAGX_MaterialSurfaceProperties()
	:
bFrictionEnabled(true),
Roughness(0.25 / (2 * 0.3)),
Viscosity(2.0 / (2.0 / 5.0E-9)), /// \todo This value is too small for Unreal's default UI slider! Make our own!
AdhesiveForce(0.0),
AdhesiveOverlap(0.0)
{
	// See agx\src\agx\Material.cpp for default values
}