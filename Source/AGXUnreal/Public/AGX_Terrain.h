#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainBarrier.h"

/// \todo Would like to not include this header, and only forward declare
/// UNiagaraSystem instead. Currently gives
///
/// Error: Unrecognized type 'UNiagaraSystem' - type must be a UCLASS, USTRUCT or UENUM
///
/// on
///
/// UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", meta = (EditCondition = "bEnableParticleRendering"))
/// UNiagaraSystem* ParticleSystemAsset;
//#include "NiagaraComponent.h"
//#include "NiagaraEmitterInstance.h"
//#include "NiagaraFunctionLibrary.h"
//#include "NiagaraSystemInstance.h"

#include "Engine/TextureRenderTarget2D.h"

#include "AGX_Terrain.generated.h"

class ALandscape;
// class UComponent;
// class UNiagaraSystem;

UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API AAGX_Terrain : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAGX_Terrain();

	/**
	 * The Landscape that AGX Terrain will use as initialization data, and will also modify
	 * in-game using a Displacement Map.
	 *
	 * Requirements:
	 *
	 * 1. Must use AGX Landscape Material or a derived material.
	 * 2. Uniform resolution of LandscapeComponents.
	 * 3. Only one Section per LandscapeComponent.
	 * 4. Uniform resolution of Quads per LandscapeComponent.
	 * 5. The Landscape Actor and AGX Terrain Actor must be centered at World Origin and have no rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	ALandscape* SourceLandscape;

	/** Whether the native terrain simulation should generate soild particles or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCreateParticles = true;

	/** Whether the native terrain simulation should auto-delete particles that are out of bounds. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bDeleteParticlesOutsideBounds = true;

	/**
	 * Scales the penetration force with the shovel velocity squared in the cutting
	 * direction according to: ( 1.0 + C * v^2 ).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	float PenetrationForceVelocityScaling = 0.0f;

	/**
	 * The maximum depth of the terrain, in centimeters from local origin. Should at least be
	 * deeper than the lowest height of the initial Landscape. Note that depth is defined in the
	 * direction of the inverted Z-axis, which means that usually positive values are used.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain", meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float MaxDepth = 200.0f;

/// \todo Add UAGX_TerrainMaterial.
#if 0
	/** The physical bulk, compaction, and particle properties of the Terrain. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	UAGX_TerrainMaterial* TerrainMaterial;
#endif

/// \todo Add UAGX_Material.
#if 0
	/** The physical material of the surface geometry of the Terrain. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	UAGX_Material* SurfaceMaterial;
#endif

/// \todo Add Shovels.
#if 0
	/**
	 * A list of the rigid body actors that should be used as terrain shovels.
	 *
	 * Every actor used as shovel MUST have the following components:
	 *
	 * Terrain Shovel Top Edge,
	 * Terrain Shovel Cut Edge,
	 * Terrain Shovel Cut Direction,
	 *
	 * in addition to the usual Rigid Body and Shape components.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FTerrainShovel> Shovels;
#endif

	/** Whether the height field rendering should be updated with deformation data. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableDisplacementRendering = true;

	// TODO: Should try finding this from the material automatically!
	/**
	 * The Displacement Map Render Target which AGX Terrain will write height changes to.
	 *
	 * Requirements:
	 *
	 * 1. Used as Displacement Map for the AGX Landscape Material.
	 * 2. Render Target Format must be "R16f".
	 * 3. SizeX and SizeY must equal number of Landscape Vertices in respective dimension (Quads + 1).
	 * 4. Texture Address Mode should be Clamp.
	 * 5. No Mip Maps.
	 * 6. Texture Group should preferrably be either "RenderTarget" for smooth results,
	 *    or "2D Pixels (unfiltered)" for more precise results.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", meta = (EditCondition = "bEnableDisplacementRendering"))
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * Rough estimation of number of particles that will exist at once. Should not be too low,
	 * or some particles might not be rendered! Used internally to allocate large enough rendering data
	 * buffers. The actual buffer sizes might have slightly larger capacity though due to data layout.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		meta =
			(EditCondition = "bEnableParticleRendering", ClampMin = "1", UIMin = "1", ClampMax = "4096",
			 UIMax = "4096"))
	int32 MaxNumRenderParticles = 2048;

	/// \todo Add UNiagaraSystem once we get particle reading from AGX Dynamics in place.
	// UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", meta = (EditCondition = "bEnableParticleRendering"))
	// UNiagaraSystem* ParticleSystemAsset;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", meta = (EditCondition = "bEnableParticleRendering"))
	UTextureRenderTarget2D* TerrainParticlesDataMap; // TODO: Should try find or create this automatically!

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Debug Rendering")
	bool bEnableActiveZoneRendering = false;

	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FTerrainBarrier NativeBarrier;

	// Height field related variables.
	TArray<FFloat16> DisplacementData;
	TArray<FUpdateTextureRegion2D> DisplacementMapRegions; // TODO: Remove!
	bool DisplacmentMapIsInitialized = false;

/// \todo Cannot use AGX Dynamics types in the AGXUnreal module. Must live in the Barrier.
#if 0
	agxCollide::HeightFieldRef InitialTerrainHeights = nullptr;
#endif

	// Particle related variables.
	TArray<FFloat32> TerrainParticlesData;
	TArray<FUpdateTextureRegion2D> ParticlesDataMapRegions; // TODO: Remove!
	bool ParticleSystemIsInitialized = false;
	/// \todo Add UNiagaraComponent once we get particle reading from AGX Dynamics in place.
	// UNiagaraComponent* ParticleSystemComponent = nullptr;
	const int32 NumPixelsPerParticle = 2;
};
