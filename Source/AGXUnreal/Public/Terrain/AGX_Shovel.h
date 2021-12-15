// Copyright 2021, Algoryx Simulation AB.


#pragma once

#include "CoreMinimal.h"

#include "AGX_Shovel.generated.h"

class FShovelBarrier;

USTRUCT()
struct AGXUNREAL_API FAGX_Shovel
{
	/// /todo Expose more variables of the native shovel API!
	/// /todo Consider making this a stand-alone Object/ActorComponent?

	GENERATED_USTRUCT_BODY()

	/**
	 * The rigid body actor that should be used as terrain shovel.
	 *
	 * Every actor MUST have the following components:
	 *
	 * Terrain Shovel Top Edge,
	 * Terrain Shovel Cut Edge,
	 * Terrain Shovel Cut Direction,
	 *
	 * in addition to the usual Rigid Body and Shape components.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	AActor* RigidBodyActor;

	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	FString BodyName;

	/**
	 * Vertical distance under the blade cutting edge that the soil is allowed
	 * to instantly merge up to [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	double VerticalBladeSoilMergeDistance = 40.0f;

	/**
	 * Extension outside the shovel bounding box where soil particle merging
	 * is forbidden [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	double NoMergeExtensionDistance = 50.0f;

	/**
	 * Linear scaling coefficient for the penetration force that the terrain will
	 * generated on this shovel.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	double PenetrationForceScaling = 1.0f;

	/**
	 * Determines if shovel <-> terrain contact should always be removed
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shovel")
	bool AlwaysRemoveShovelContacts = false;

	static void UpdateNativeShovelProperties(
		FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel);
};
