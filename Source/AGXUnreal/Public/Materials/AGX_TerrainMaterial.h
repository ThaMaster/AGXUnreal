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
 * Terrain or Contact Material that referenced the UAGX_TerrainMaterial asset will swap its
 * reference to the in-game created instance instead. This means that ultimately only
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

	virtual void SetRoughness_BP(float Roughness) override;
	virtual float GetRoughness_BP() const override;

	virtual void SetRoughness(double Roughness) override;
	virtual double GetRoughness() const override;

	virtual void SetSurfaceViscosity_BP(float Viscosity) override;
	virtual float GetSurfaceViscosity_BP() const override;

	virtual void SetSurfaceViscosity(double Viscosity) override;
	virtual double GetSurfaceViscosity() const override;

	virtual void SetAdhesion_BP(float AdhesiveForce, float AdhesiveOverlap) override;
	virtual float GetAdhesiveForce_BP() const override;
	virtual float GetAdhesiveOverlap_BP() const override;

	virtual void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap) override;
	virtual double GetAdhesiveForce() const override;
	virtual double GetAdhesiveOverlap() const override;

	// Bulk properties.

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainBulkProperties TerrainBulk;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Adhesion Overlap Factor"))
	virtual void SetAdhesionOverlapFactor_BP(float AdhesionOverlapFactor);

	virtual void SetAdhesionOverlapFactor(double AdhesionOverlapFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Adhesion Overlap Factor"))
	virtual float GetAdhesionOverlapFactor_BP() const;

	virtual double GetAdhesionOverlapFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Cohesion"))
	virtual void SetCohesion_BP(float Cohesion);

	virtual void SetCohesion(double Cohesion);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Cohesion"))
	virtual float GetCohesion_BP() const;

	virtual double GetCohesion() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		MEta = (DisplayName = "Set Density"))
	virtual void SetDensity_BP(float Density);

	virtual void SetDensity(double Density);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Density"))
	virtual float GetDensity_BP() const;

	virtual double GetDensity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Dilatancy Angle"))
	virtual void SetDilatancyAngle_BP(float DilatancyAngle);

	virtual void SetDilatancyAngle(double DilatancyAngle);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Dilatancy Angle "))
	virtual float GetDilatancyAngle_BP() const;

	virtual double GetDilatancyAngle() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Friction Angle"))
	virtual void SetFrictionAngle_BP(float FrictionAngle);

	virtual void SetFrictionAngle(double FrictionAngle);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Friction Angle"))
	virtual float GetFrictionAngle_BP() const;

	virtual double GetFrictionAngle() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Max Density"))
	virtual void SetMaxDensity_BP(float MaxDensity);

	virtual void SetMaxDensity(double MaxDensity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Max Density"))
	virtual float GetMaxDensity_BP() const;

	virtual double GetMaxDensity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Poissons Ratio"))
	virtual void SetPoissonsRatio_BP(float PoissonsRatio);

	virtual void SetPoissonsRatio(double PoissonsRatio);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Poissons Ratio"))
	virtual float GetPoissonsRatio_BP() const;

	virtual double GetPoissonsRatio() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Swell Factor"))
	virtual void SetSwellFactor_BP(float SwellFactor);

	virtual void SetSwellFactor(double SwellFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Swell Factor"))
	virtual float GetSwellFactor_BP() const;

	virtual double GetSwellFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Youngs Modulus"))
	virtual void SetYoungsModulus_BP(float YoungsModulus);

	virtual void SetYoungsModulus(double YoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Youngs Modulus"))
	virtual float GetYoungsModulus_BP() const;

	virtual double GetYoungsModulus() const;

	// Compaction properties.

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainCompactionProperties TerrainCompaction;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Angle Of Repose Compaction Rate"))
	virtual void SetAngleOfReposeCompactionRate_BP(float AngleOfReposeCompactionRate);

	virtual void SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Angle Of Repose Compaction Rate"))
	virtual float GetAngleOfReposeCompactionRate_BP() const;

	virtual double GetAngleOfReposeCompactionRate() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Bank State Phi"))
	virtual void SetBankStatePhi_BP(float Phi0);

	virtual void SetBankStatePhi(double Phi0);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Bank State Phi"))
	virtual float GetBankStatePhi_BP() const;

	virtual double GetBankStatePhi() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Compation Time Relaxation Constraint"))
	virtual void SetCompactionTimeRelaxationConstant_BP(float CompactionTimeRelaxationConstant);

	virtual void SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Compaction Time Relaxation Constant"))
	virtual float GetCompactionTimeRelaxationConstant_BP() const;

	virtual double GetCompactionTimeRelaxationConstant() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Compression Index"))
	virtual void SetCompressionIndex_BP(float CompressionIndex);

	virtual void SetCompressionIndex(double CompressionIndex);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Compression Index"))
	virtual float GetCompressionIndex_BP() const;

	virtual double GetCompressionIndex() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Hardening Constant Ke"))
	virtual void SetHardeningConstantKe_BP(float K_e);

	virtual void SetHardeningConstantKe(double K_e);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Hardening Constant Ke"))
	virtual float GetHardeningConstantKe_BP() const;

	virtual double GetHardeningConstantKe() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Hardening Constant Ne"))
	virtual void SetHardeningConstantNe_BP(float N_e);

	virtual void SetHardeningConstantNe(double N_e);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Hardening Constant Ne"))
	virtual float GetHardeningConstantNe_BP() const;

	virtual double GetHardeningConstantNe() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Preconsolidation Stress"))
	virtual void SetPreconsolidationStress_BP(float PreconsolidationStress);

	virtual void SetPreconsolidationStress(double PreconsolidationStress);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Preconsolidation Stress"))
	virtual float GetPreconsolidationStress_BP() const;

	virtual double GetPreconsolidationStress() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Stress Cut Off Fraction"))
	virtual void SetStressCutOffFraction_BP(float StressCutOffFraction);

	virtual void SetStressCutOffFraction(double StressCutOffFraction);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Stress Cut Off Fraction"))
	virtual float GetStressCutOffFraction_BP() const;

	virtual double GetStressCutOffFraction() const;

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
