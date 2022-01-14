// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"

#include "AGX_TerrainBulkProperties.generated.h"

/**
 * Terrain specific physical properties.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainBulkProperties
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * Sets the adhesion overlap factor of the bulk material, i.e what fraction of the particle
	 * radius is allowed to overlap to simulate adhesion.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double AdhesionOverlapFactor;

	/**
	 * Sets the bulk cohesion of the bulk material [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Cohesion;

	/**
	 * Density of the terrain bulk [kg/m^3].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Density;

	/**
	 * Sets the dilatancy angle of the bulk material [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Bulk")
	double DilatancyAngle;

	/**
	 * Sets the internal friction angle of the bulk material [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Bulk")
	double FrictionAngle;

	/**
	 * Sets the maximum density of the bulk material [kg/m^3].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double MaxDensity;

	/**
	 * Sets the Poisson's ratio of the bulk material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double PoissonsRatio;

	/**
	 * Sets the swell factor of the material, i.e how much the material will expand during
	 * excavation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double SwellFactor;

	/**
	 * Sets the bulk Young's modulus of the bulk material [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulus;

public:
	FAGX_TerrainBulkProperties();
};
