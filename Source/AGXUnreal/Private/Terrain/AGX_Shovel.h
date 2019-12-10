#pragma once

#include "CoreMinimal.h"
#include "AGX_Shovel.generated.h"

USTRUCT()
struct FAGX_Shovel
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
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	AActor* RigidBodyActor;

	/**
	 * Vertical distance under the blade cutting edge that the soil is allowed
	 * to instantly merge up to, in centimeters.
	 */
	UPROPERTY(EditAnywhere)
	float VerticalBladeSoilMergeDistance = 40.0f;

	/**
     * Extension outside the shovel bounding box where soil particle merging
	 * is forbidden, in centimeters.
	 */
	UPROPERTY(EditAnywhere)
	float NoMergeExtensionDistance = 50.0f;

	/**
	 * Linear scaling coefficient for the penetration force that the terrain will
	 * generated on this shovel.
	 */
	UPROPERTY(EditAnywhere)
	float PenetrationForceScaling = 1.0f;
};
