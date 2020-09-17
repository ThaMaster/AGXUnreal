#pragma once

// AGXUnreal includes.
#include "Terrain/TerrainBarrier.h"
#include "Terrain/AGX_Shovel.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Array.h"

/// \todo Would like to not include this header, and only forward declare
/// UNiagaraSystem instead. Currently gives
///
/// Error: Unrecognized type 'UNiagaraSystem' - type must be a UCLASS, USTRUCT or UENUM
///
/// on
///
/// UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering", meta = (EditCondition =
/// "bEnableParticleRendering")) UNiagaraSystem* ParticleSystemAsset;
//#include "NiagaraComponent.h"
//#include "NiagaraEmitterInstance.h"
//#include "NiagaraFunctionLibrary.h"
//#include "NiagaraSystemInstance.h"

#include "Engine/TextureRenderTarget2D.h"

#include "AGX_Terrain.generated.h"

class UAGX_TerrainMaterialBase;
class ALandscape;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX")
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
	 * 5. The Landscape Actor and AGX Terrain Actor must be centered at World Origin and have no
	 * rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	ALandscape* SourceLandscape;

	/** Whether the native terrain simulation should generate soild particles or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCreateParticles = true;

	/** Whether the native terrain simulation should auto-delete particles that are out of bounds.
	 */
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
		EditAnywhere, Category = "AGX Terrain",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "1000", UIMax = "1000"))
	float MaxDepth = 200.0f;


	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	UAGX_TerrainMaterialBase* TerrainMaterial;


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
	TArray<FAGX_Shovel> Shovels;

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
	 * 3. SizeX and SizeY must equal number of Landscape Vertices in respective dimension (Quads +
	 * 1).
	 * 4. Texture Address Mode should be Clamp.
	 * 5. No Mip Maps.
	 * 6. Texture Group should preferrably be either "RenderTarget" for smooth results,
	 *    or "2D Pixels (unfiltered)" for more precise results.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		meta = (EditCondition = "bEnableDisplacementRendering"))
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * Rough estimation of number of particles that will exist at once. Should not be too low,
	 * or some particles might not be rendered! Used internally to allocate large enough rendering
	 * data buffers. The actual buffer sizes might have slightly larger capacity though due to data
	 * layout.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		meta =
			(EditCondition = "bEnableParticleRendering", ClampMin = "1", UIMin = "1",
			 ClampMax = "4096", UIMax = "4096"))
	int32 MaxNumRenderParticles = 2048;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset;

	// \todo Should try to find or create this automatically!
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		meta = (EditCondition = "bEnableParticleRendering"))
	UTextureRenderTarget2D* TerrainParticlesDataMap;

	/** Whether shovel active zone should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Debug Rendering")
	bool bEnableActiveZoneRendering = false;

	/// Return true if the AGX Dynamics object has been created. False otherwise.
	bool HasNative();

	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void InitializeNative();
	void CreateNativeTerrain();
	void CreateNativeShovels();
	void CreateTerrainMaterial();

	void SetInitialTransform();
	void InitializeRendering();
	void InitializeDisplacementMap();
	void UpdateDisplacementMap();
	void ClearDisplacementMap();
	bool InitializeParticleSystem();
	bool InitializeParticleSystemComponent();
	bool InitializeParticlesMap();
	void UpdateParticlesMap();
	void ClearParticlesMap();

private:
	FTerrainBarrier NativeBarrier;

	// Height field related variables.
	TArray<float> OriginalHeights;
	TArray<FFloat16> DisplacementData;
	TArray<FUpdateTextureRegion2D> DisplacementMapRegions; // TODO: Remove!
	bool DisplacementMapInitialized = false;

/// \todo Cannot use AGX Dynamics types in the AGXUnreal module. Must live in the Barrier.
#if 0
	agxCollide::HeightFieldRef InitialTerrainHeights = nullptr;
#endif

	// Particle related variables.
	TArray<FFloat32> TerrainParticlesData;
	TArray<FUpdateTextureRegion2D> ParticlesDataMapRegions; // TODO: Remove!
	bool ParticleSystemInitialized = false;
	UNiagaraComponent* ParticleSystemComponent = nullptr;
	const int32 NumPixelsPerParticle = 2;
};
