// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Terrain/AGX_ShovelExcavationSettings.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_ShovelProperties.generated.h"

/**
 * An asset used to hold configuration properties for Shovel Components.
 */
UCLASS(ClassGroup = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_ShovelProperties : public UObject
{
	GENERATED_BODY()

public:
	// @todo Add a bunch of Barrier-aware setter and getter functions.

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool Enable;

	UPROPERTY(EditAnywhere, Category = "Contacts")
	bool AlwaysRemoveShovelContacts {false};

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableInnerShapeCreateDynamicMass;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleForceFeedback;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	bool EnableParticleFreeDeformers;

	// @todo Excavation settings.

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MinimumSubmergedContactLengthFraction;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real VerticalBladeSoilMergeDistance;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real NoMergeExtensionDistance;

	/**
	 * Number of teeth of the Shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "Teeth")
	uint32 NumberOfTeeth;

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real ToothLength;

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real MaximumToothRadius;

	UPROPERTY(EditAnywhere, Category = "Teeth")
	FAGX_Real MinimumToothRadius;

	// @todo Should Paging Terrain settings be here or somewhere else?
	// The other option is in the Terrain's Shovels list, which is currently just a list of Shovel
	// References. That would need to become a struct again, if so. Do we expect the need for
	// shovels with varying radii depending on the scene?

	/**
	 * The max distance from the Shovel at which new Terrain Tiles is guaranteed to be loaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "Paging Terrain")
	FAGX_Real RequiredRadius {600.f};

	/**
	 * The max distance from the Shovel at which new Terrain Tiles will be preloaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "Paging Terrain")
	FAGX_Real PreloadRadius {1000.f};

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real ParticleInclusionMultiplier;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationDepthThreshold;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real PenetrationForceScaling;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real MaximumPenetrationForce;

	UPROPERTY(EditAnywhere, Category = "Shovel")
	FAGX_Real SecondarySeparationDeadloadLimit;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings PrimaryExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformBackExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformRightExcavationSettings;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FAGX_ShovelExcavationSettings DeformLeftExcavationSettings;

	// @todo Soil Penetration Model.
	// See
	// https://www.algoryx.se/documentation/complete/agx/html/doc/html/classagxTerrain_1_1Shovel.html#a5686243f6a966d59b17b127a83c3a88a
};
