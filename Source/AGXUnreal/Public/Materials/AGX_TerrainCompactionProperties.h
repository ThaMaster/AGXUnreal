#pragma once

#include "CoreMinimal.h"

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
	UPROPERTY(EditAnywhere)
	double AngleOfReposeCompactionRate;

	/**
	 * Sets the phi0 value of the bank state soil.
	 */
	UPROPERTY(EditAnywhere)
	double Phi0;

	/**
	 * Sets time relaxation for compaction.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double CompactionTimeRelaxationConstant;

	/**
	 * Sets the compression index for the soil, which is the constant that determines how fast the
	 * soil should compress given increased surface stress.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double CompressionIndex;

	/**
	 * Sets the hardening constant k_e of the bulk material, i.e how the Young's modulus of the
	 * terrain contacts should scale with increasing/decreasing compaction.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double K_e;

	/**
	 * Sets the hardening constant n_e of the bulk material, i.e how the Young's modulus  of the
	 * terrain contacts should scale with increasing/decreasing compaction.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double N_e;

	/**
	 * Sets the stress at which the soil in the default state was compressed in, i.e when it has
	 * nominal compaction 1.0.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double PreconsolidationStress;

	/**
	 * Set the fraction of the surface stress that should serve as a cutoff value from when the
	 * stress propagation from the surface downward into the soil should stop.
	 */
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double StressCutOffFraction;

public:
	FAGX_TerrainCompactionProperties();
};
