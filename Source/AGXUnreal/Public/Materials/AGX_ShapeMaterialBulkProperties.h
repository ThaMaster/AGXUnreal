// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGX_ShapeMaterialBulkProperties.generated.h"

/**
 * Physical properties for the bulk of Shapes using the AGX Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialBulkProperties
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * Density of Shapes using the material, in kg/m². The density can be used
	 * for automatic calculation of total mass and inertia of the Rigid Body
	 * (see mass options of Rigid Body Component).
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Density;

	/**
	 * Young's modulus of the material. Same as spring coefficient k.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulus;

	/**
	 * Bulk viscosity coefficient of the material (1.0 - restitution coefficient).
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Viscosity;

	/**
	 * Damping factor used by the contact constraint. The value is the time the
	 * contact constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Damping;

	/**
	 * Minimum elastic rest length of the contact material, in meters.
	 *
	 * This is only used if the contact area approach is used if the 'Use Contact Area Approach' is
	 * enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Bulk Properties")
	double MinElasticRestLength;

	/**
	 * Maximum elastic rest length of the contact material, in meters.
	 *
	 * This is only used if the contact area approach is used if the 'Use Contact Area Approach' is
	 * enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Bulk Properties")
	double MaxElasticRestLength;

public:
	FAGX_ShapeMaterialBulkProperties();
};
