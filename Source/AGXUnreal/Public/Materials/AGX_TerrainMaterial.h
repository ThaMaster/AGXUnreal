// Copyright 2022, Algoryx Simulation AB.

#pragma once


// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_TerrainBulkProperties.h"
#include "Materials/AGX_TerrainCompactionProperties.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

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

	void SetFrictionEnabled(bool Enabled);
	bool GetFrictionEnabled() const;

	virtual void SetRoughness_BP(float Roughness) override;
	virtual float GetRoughness_BP() const override;

	void SetRoughness(FAGX_Real Roughness);
	FAGX_Real GetRoughness() const;

	virtual void SetSurfaceViscosity_BP(float Viscosity) override;
	virtual float GetSurfaceViscosity_BP() const override;

	void SetSurfaceViscosity(FAGX_Real Viscosity);
	FAGX_Real GetSurfaceViscosity() const;

	virtual void SetAdhesion_BP(float AdhesiveForce, float AdhesiveOverlap) override;
	virtual float GetAdhesiveForce_BP() const override;
	virtual float GetAdhesiveOverlap_BP() const override;

	void SetAdhesion(FAGX_Real AdhesiveForce, FAGX_Real AdhesiveOverlap);
	FAGX_Real GetAdhesiveForce() const;
	FAGX_Real GetAdhesiveOverlap() const;

	// Bulk properties.

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainBulkProperties TerrainBulk;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetAdhesionOverlapFactor_BP(float AdhesionOverlapFactor);

	virtual void SetAdhesionOverlapFactor(FAGX_Real AdhesionOverlapFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetAdhesionOverlapFactor_BP() const;

	virtual FAGX_Real GetAdhesionOverlapFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetCohesion_BP(float Cohesion);

	virtual void SetCohesion(FAGX_Real Cohesion);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetCohesion_BP() const;

	virtual FAGX_Real GetCohesion() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetDensity_BP(float Density);

	virtual void SetDensity(FAGX_Real Density);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetDensity_BP() const;

	virtual FAGX_Real GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetDilatancyAngle_BP(float DilatancyAngle);

	virtual void SetDilatancyAngle(FAGX_Real DilatancyAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetDilatancyAngle_BP() const;

	virtual FAGX_Real GetDilatancyAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetFrictionAngle_BP(float FrictionAngle);

	virtual void SetFrictionAngle(FAGX_Real FrictionAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetFrictionAngle_BP() const;

	virtual FAGX_Real GetFrictionAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetMaxDensity_BP(float MaxDensity);

	virtual void SetMaxDensity(FAGX_Real MaxDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetMaxDensity_BP() const;

	virtual FAGX_Real GetMaxDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetPoissonsRatio_BP(float PoissonsRatio);

	virtual void SetPoissonsRatio(FAGX_Real PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetPoissonsRatio_BP() const;

	virtual FAGX_Real GetPoissonsRatio() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetSwellFactor_BP(float SwellFactor);

	virtual void SetSwellFactor(FAGX_Real SwellFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetSwellFactor_BP() const;

	virtual FAGX_Real GetSwellFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual void SetYoungsModulus_BP(float YoungsModulus);

	virtual void SetYoungsModulus(FAGX_Real YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	virtual float GetYoungsModulus_BP() const;

	virtual FAGX_Real GetYoungsModulus() const;

	// Compaction properties.

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainCompactionProperties TerrainCompaction;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetAngleOfReposeCompactionRate_BP(float AngleOfReposeCompactionRate);

	virtual void SetAngleOfReposeCompactionRate(FAGX_Real AngleOfReposeCompactionRate);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetAngleOfReposeCompactionRate_BP() const;

	virtual FAGX_Real GetAngleOfReposeCompactionRate() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetBankStatePhi_BP(float Phi0);

	virtual void SetBankStatePhi(FAGX_Real Phi0);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetBankStatePhi_BP() const;

	virtual FAGX_Real GetBankStatePhi() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetCompactionTimeRelaxationConstant_BP(float CompactionTimeRelaxationConstant);

	virtual void SetCompactionTimeRelaxationConstant(FAGX_Real CompactionTimeRelaxationConstant);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetCompactionTimeRelaxationConstant_BP() const;

	virtual FAGX_Real GetCompactionTimeRelaxationConstant() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetCompressionIndex_BP(float CompressionIndex);

	virtual void SetCompressionIndex(FAGX_Real CompressionIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetCompressionIndex_BP() const;

	virtual FAGX_Real GetCompressionIndex() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetHardeningConstantKe_BP(float K_e);

	virtual void SetHardeningConstantKe(FAGX_Real K_e);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetHardeningConstantKe_BP() const;

	virtual FAGX_Real GetHardeningConstantKe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetHardeningConstantNe_BP(float N_e);

	virtual void SetHardeningConstantNe(FAGX_Real N_e);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetHardeningConstantNe_BP() const;

	virtual FAGX_Real GetHardeningConstantNe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetPreconsolidationStress_BP(float PreconsolidationStress);

	virtual void SetPreconsolidationStress(FAGX_Real PreconsolidationStress);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetPreconsolidationStress_BP() const;

	virtual FAGX_Real GetPreconsolidationStress() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual void SetStressCutOffFraction_BP(float StressCutOffFraction);

	virtual void SetStressCutOffFraction(FAGX_Real StressCutOffFraction);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	virtual float GetStressCutOffFraction_BP() const;

	virtual FAGX_Real GetStressCutOffFraction() const;

	void CopyFrom(const FTerrainMaterialBarrier& Source);

	FTerrainMaterialBarrier* GetOrCreateTerrainMaterialNative(UWorld* PlayingWorld);
	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;
	static UAGX_TerrainMaterial* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterial* Source);

	void CopyTerrainMaterialProperties(const UAGX_TerrainMaterial* Source);

	bool IsInstance() const override;

private:

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
