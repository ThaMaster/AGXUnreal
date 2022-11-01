// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"

#include "AGX_TerrainCompactionProperties.generated.h"

/**
 * Terrain compaction physical properties.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainCompactionProperties
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * Sets how the compaction should increase the angle of repose.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Compaction")
	FAGX_Real AngleOfReposeCompactionRate;

	/**
	 * Sets the phi0 value of the bank state soil.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Compaction")
	FAGX_Real Phi0;

	/**
	 * Sets time relaxation for compaction.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real CompactionTimeRelaxationConstant;

	/**
	 * Sets the compression index for the soil, which is the constant that determines how fast the
	 * soil should compress given increased surface stress.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real CompressionIndex;

	/**
	 * Sets the hardening constant k_e of the bulk material, i.e how the Young's modulus of the
	 * terrain contacts should scale with increasing/decreasing compaction.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real K_e;

	/**
	 * Sets the hardening constant n_e of the bulk material, i.e how the Young's modulus  of the
	 * terrain contacts should scale with increasing/decreasing compaction.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real N_e;

	/**
	 * Sets the stress at which the soil in the default state was compressed in, i.e when it has
	 * nominal compaction 1.0 [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real PreconsolidationStress;

	/**
	 * Set the fraction of the surface stress that should serve as a cutoff value from when the
	 * stress propagation from the surface downward into the soil should stop.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Compaction",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real StressCutOffFraction;

	/**
	 * Set the dilatancy angle scaling factor with respect to compaction in the terrain. The
	 * dilatancy angle modifies the effective local angle of friction in terrain by the following
	 * formula:
	 * eff_friction_angle = friction_angle + scalingFactor * ( voxelCompaction -
	 * criticalCompaction )
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Compaction")
	FAGX_Real DilatancyAngleScalingFactor {20.0 * PI / 180};

public:
	FAGX_TerrainCompactionProperties();
};
