// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGX_MaterialSurfaceProperties.generated.h"

/**
 * Physical properties for the surface of Shapes using the AGX Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_MaterialSurfaceProperties
{
	GENERATED_USTRUCT_BODY()

public:

	/**
	 * Specify if the friction should be used when solving contacts for this Material.
	 * If this is set to false, the solver will NOT calculate any friction when this material
	 * is in contact with another material.
	 */
	UPROPERTY(EditAnywhere)
	bool bFrictionEnabled;

	/**
	 * Unitless roughness parameter used to calculate the final friction coefficient.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bFrictionEnabled"))
	double Roughness;

	/**
	 * Surface viscosity parameter telling how dry/wet the surface is. For larger values,
	 * the surface is wetter, and contacting objects will creep more. It's like compliance
	 * for the friction constraints.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Viscosity;

	/**
	 * The attractive force between two colliding objects.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double AdhesiveForce;

	/**
	 * Allowed overlap (length >= 0) from surface for resting contact. At this overlap,
	 * no adhesive force is applied. At lower overlap, the adhesion force will work,
	 * at higher overlap, the (usual) contact force.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double AdhesiveOverlap;

public:

	FAGX_MaterialSurfaceProperties();

};
