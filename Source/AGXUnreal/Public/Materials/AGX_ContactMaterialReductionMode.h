// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_ContactMaterialEnums.h"
#include "AGX_ContactMaterialReductionMode.generated.h"

/**
 * Specifies to what extent contact reduction will be used.
 */
UENUM()
enum class EAGX_ContactReductionMode
{
	/** No contact reduction enabled. */
	None,

	/** Reduce contacts between geometries. */
	Geometry,

	/** Two step reduction: first between geometries, and then between rigid bodies. */
	All
};

/**
 * Contact reduction mode properties of the AGX Contact Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ContactMaterialReductionMode
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * Whether contact reduction should be enabled and to what extent.
	 *
	 * By using contact reduction, the number of contact points later submitted to the solver as contact constraint
	 * can be heavily reduced, hence improving performance.
	 */
	UPROPERTY(EditAnywhere)
	EAGX_ContactReductionMode Mode;

	/**
	 * The resolution used when evaluating contacts for reduction between geometry contacts. A high value will keep
	 * more contacts, lower will result in more aggressive reduction the default. Commonly a value of 2 or 3 will
	 * give good result.
	 *
	 * Zero means that the value will be overridden by the general AGX Space setting.
	 *
	 * Not used if the 'Contact Reduction Mode' property is set to 'None'.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
	uint8 BinResolution; /// \todo Disable if Mode is set to 'None'.

public:
	FAGX_ContactMaterialReductionMode();
};