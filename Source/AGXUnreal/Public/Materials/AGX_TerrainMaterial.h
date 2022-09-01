// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_TerrainBulkProperties.h"
#include "Materials/AGX_TerrainCompactionProperties.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/TerrainMaterialBarrier.h"

#include "AGX_TerrainMaterial.generated.h"


/**
 * Defines the material for a terrain. Affects both surface and bulk properties.
 *
 * Terrain Materials are created by the user in-Editor by creating a UAGX_TerrainMaterial asset.
 * In-Editor they are treated as assets and can be referenced by either Terrains or Contact
 * Materials.
 *
 * When game begins playing, one UAGX_TerrainMaterial instace will be created for each
 * UAGX_TerrainMaterial asset that is referenced by an in-game Terrain or Contact Material. The
 * UAGX_TerrainMaterial will create the actual native AGX terrain material. The in-game
 * Terrain or Contact Material that referenced the UAGX_TerrainMaterial asset will swap its reference
 * to the in-game created instance instead. This means that ultimately only
 * UAGX_TerrainMaterials will be referenced in-game. When play stops the in-Editor state
 * will be returned.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_TerrainMaterial : public UAGX_MaterialBase
{
public:
	/**
	 * The terrain material instance can spawn both a TerrainMaterialBarrier (affecting the bulk
	 * properties of the terrain) and a ShapeMaterialBarrier (affecting the surface properties).
	 * This translates directly to AGX Dynamics' native types TerrainMaterial and Material.
	 */

	GENERATED_BODY()

	// Surface properties.
	virtual void SetFrictionEnabled(bool Enabled) override;
	virtual bool GetFrictionEnabled() const override;

	virtual void SetRoughness(float Roughness) override;
	virtual float GetRoughness() const override;

	virtual void SetSurfaceViscosity(float Viscosity) override;
	virtual float GetSurfaceViscosity() const override;

	virtual void SetAdhesion(float AdhesiveForce, float AdhesiveOverlap) override;
	virtual float GetAdhesiveForce() const override;
	virtual float GetAdhesiveOverlap() const override;

	// Bulk properties.
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainBulkProperties TerrainBulk;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetAdhesionOverlapFactor(float AdhesionOverlapFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetAdhesionOverlapFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetCohesion(float Cohesion);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetCohesion() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetDensity(float Density);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetDilatancyAngle(float DilatancyAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetDilatancyAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetFrictionAngle(float FrictionAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetFrictionAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetMaxDensity(float MaxDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetMaxDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetPoissonsRatio(float PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetPoissonsRatio() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetSwellFactor(float SwellFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetSwellFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetYoungsModulus(float YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetYoungsModulus() const;

	// Compaction properties.
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainCompactionProperties TerrainCompaction;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetAngleOfReposeCompactionRate() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetBankStatePhi(float Phi0);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetBankStatePhi() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetCompactionTimeRelaxationConstant(float CompactionTimeRelaxationConstant);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetCompactionTimeRelaxationConstant() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetCompressionIndex(float CompressionIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetCompressionIndex() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetHardeningConstantKe(float K_e);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetHardeningConstantKe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetHardeningConstantNe(float N_e);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetHardeningConstantNe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetPreconsolidationStress(float PreconsolidationStress);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetPreconsolidationStress() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetStressCutOffFraction(float StressCutOffFraction);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetStressCutOffFraction() const;

	void CopyFrom(const FTerrainMaterialBarrier& Source);

	FTerrainMaterialBarrier* GetOrCreateTerrainMaterialNative(UWorld* PlayingWorld);
	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;
	static UAGX_TerrainMaterial* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterial* Source);

	void CopyTerrainMaterialProperties(const UAGX_TerrainMaterial* Source);

private:
	bool IsAssetAGX() const override;
	bool IsInstanceAGX() const override;

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CreateTerrainMaterialNative(UWorld* PlayingWorld);
	void CreateShapeMaterialNative(UWorld* PlayingWorld);

	bool HasTerrainMaterialNative() const;
	bool HasShapeMaterialNative() const;

	FTerrainMaterialBarrier* GetTerrainMaterialNative();
	FShapeMaterialBarrier* GetShapeMaterialNative();

	void UpdateTerrainMaterialNativeProperties();
	void UpdateShapeMaterialNativeProperties();

	TWeakObjectPtr<UAGX_TerrainMaterial> Asset;
	TWeakObjectPtr<UAGX_TerrainMaterial> Instance;
	TUniquePtr<FTerrainMaterialBarrier> TerrainMaterialNativeBarrier;
	TUniquePtr<FShapeMaterialBarrier> ShapeMaterialNativeBarrier;
};
