// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/AGX_TerrainBulkProperties.h"
#include "Materials/AGX_TerrainCompactionProperties.h"
#include "Materials/AGX_TerrainExcavationContactProperties.h"
#include "Materials/AGX_TerrainParticleProperties.h"
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
	void SetAdhesionOverlapFactor_BP(float AdhesionOverlapFactor);

	void SetAdhesionOverlapFactor(double AdhesionOverlapFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Adhesion Overlap Factor"))
	float GetAdhesionOverlapFactor_BP() const;

	double GetAdhesionOverlapFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Cohesion"))
	void SetCohesion_BP(float Cohesion);

	void SetCohesion(double Cohesion);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Cohesion"))
	float GetCohesion_BP() const;

	double GetCohesion() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		MEta = (DisplayName = "Set Density"))
	void SetDensity_BP(float Density);

	void SetDensity(double Density);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Density"))
	float GetDensity_BP() const;

	double GetDensity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Dilatancy Angle"))
	void SetDilatancyAngle_BP(float DilatancyAngle);

	void SetDilatancyAngle(double DilatancyAngle);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Dilatancy Angle "))
	float GetDilatancyAngle_BP() const;

	double GetDilatancyAngle() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Friction Angle"))
	void SetFrictionAngle_BP(float FrictionAngle);

	void SetFrictionAngle(double FrictionAngle);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Friction Angle"))
	float GetFrictionAngle_BP() const;

	double GetFrictionAngle() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Max Density"))
	void SetMaxDensity_BP(float MaxDensity);

	void SetMaxDensity(double MaxDensity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Max Density"))
	float GetMaxDensity_BP() const;

	double GetMaxDensity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Poissons Ratio"))
	void SetPoissonsRatio_BP(float PoissonsRatio);

	void SetPoissonsRatio(double PoissonsRatio);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Poissons Ratio"))
	float GetPoissonsRatio_BP() const;

	double GetPoissonsRatio() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Swell Factor"))
	void SetSwellFactor_BP(float SwellFactor);

	void SetSwellFactor(double SwellFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Swell Factor"))
	float GetSwellFactor_BP() const;

	double GetSwellFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Set Youngs Modulus"))
	void SetYoungsModulus_BP(float YoungsModulus);

	void SetYoungsModulus(double YoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Bulk",
		Meta = (DisplayName = "Get Youngs Modulus"))
	float GetYoungsModulus_BP() const;

	double GetYoungsModulus() const;

	// Compaction properties.

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainCompactionProperties TerrainCompaction;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Angle Of Repose Compaction Rate"))
	void SetAngleOfReposeCompactionRate_BP(float AngleOfReposeCompactionRate);

	void SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Angle Of Repose Compaction Rate"))
	float GetAngleOfReposeCompactionRate_BP() const;

	double GetAngleOfReposeCompactionRate() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Bank State Phi"))
	void SetBankStatePhi_BP(float Phi0);

	void SetBankStatePhi(double Phi0);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Bank State Phi"))
	float GetBankStatePhi_BP() const;

	double GetBankStatePhi() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Compation Time Relaxation Constraint"))
	void SetCompactionTimeRelaxationConstant_BP(float CompactionTimeRelaxationConstant);

	void SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Compaction Time Relaxation Constant"))
	float GetCompactionTimeRelaxationConstant_BP() const;

	double GetCompactionTimeRelaxationConstant() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Compression Index"))
	void SetCompressionIndex_BP(float CompressionIndex);

	void SetCompressionIndex(double CompressionIndex);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Compression Index"))
	float GetCompressionIndex_BP() const;

	double GetCompressionIndex() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Hardening Constant Ke"))
	void SetHardeningConstantKe_BP(float Ke);

	void SetHardeningConstantKe(double Ke);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Hardening Constant Ke"))
	float GetHardeningConstantKe_BP() const;

	double GetHardeningConstantKe() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Hardening Constant Ne"))
	void SetHardeningConstantNe_BP(float Ne);

	void SetHardeningConstantNe(double Ne);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Hardening Constant Ne"))
	float GetHardeningConstantNe_BP() const;

	double GetHardeningConstantNe() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Preconsolidation Stress"))
	void SetPreconsolidationStress_BP(float PreconsolidationStress);

	void SetPreconsolidationStress(double PreconsolidationStress);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Preconsolidation Stress"))
	float GetPreconsolidationStress_BP() const;

	double GetPreconsolidationStress() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Stress Cut Off Fraction"))
	void SetStressCutOffFraction_BP(float StressCutOffFraction);

	void SetStressCutOffFraction(double StressCutOffFraction);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Stress Cut Off Fraction"))
	float GetStressCutOffFraction_BP() const;

	double GetStressCutOffFraction() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Set Dilatancy Angle Scaling Factor"))
	void SetDilatancyAngleScalingFactor_BP(float DilatancyAngleScalingFactor);

	void SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Dilatancy Angle Scaling Factor"))
	float GetDilatancyAngleScalingFactor_BP() const;

	double GetDilatancyAngleScalingFactor() const;

	// Particle properties.
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainParticleProperties TerrainParticles;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Adhesion Overlap Factor"))
	void SetParticleAdhesionOverlapFactor_BP(float ParticleAdhesionOverlapFactor);

	void SetParticleAdhesionOverlapFactor(double ParticleAdhesionOverlapFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Adhesion Overlap Factor"))
	float GetParticleAdhesionOverlapFactor_BP() const;

	double GetParticleAdhesionOverlapFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Cohesion"))
	void SetParticleCohesion_BP(float ParticleCohesion);

	void SetParticleCohesion(double ParticleCohesion);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Cohesion"))
	float GetParticleCohesion_BP() const;

	double GetParticleCohesion() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Restitution"))
	void SetParticleRestitution_BP(float ParticleRestitution);

	void SetParticleRestitution(double ParticleRestitution);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Restitution"))
	float GetParticleRestitution_BP() const;

	double GetParticleRestitution() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Rolling Resistance"))
	void SetParticleRollingResistance_BP(float ParticleRollingResistance);

	void SetParticleRollingResistance(double ParticleRollingResistance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Rolling Resistance"))
	float GetParticleRollingResistance_BP() const;

	double GetParticleRollingResistance() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Surface Friction"))
	void SetParticleSurfaceFriction_BP(float ParticleSurfaceFriction);

	void SetParticleSurfaceFriction(double ParticleSurfaceFriction);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Surface Friction"))
	float GetParticleSurfaceFriction_BP() const;

	double GetParticleSurfaceFriction() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Terrain Cohesion"))
	void SetParticleTerrainCohesion_BP(float ParticleTerrainCohesion);

	void SetParticleTerrainCohesion(double ParticleTerrainCohesion);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Terrain Cohesion"))
	float GetParticleTerrainCohesion_BP() const;

	double GetParticleTerrainCohesion() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Terrain Restitution"))
	void SetParticleTerrainRestitution_BP(float ParticleTerrainRestitution);

	void SetParticleTerrainRestitution(double ParticleTerrainRestitution);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Terrain Restitution"))
	float GetParticleTerrainRestitution_BP() const;

	double GetParticleTerrainRestitution() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Terrain Rolling Resistance"))
	void SetParticleTerrainRollingResistance_BP(float ParticleTerrainRollingResistance);

	void SetParticleTerrainRollingResistance(double ParticleTerrainRollingResistance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Terrain Rolling Resistance"))
	float GetParticleTerrainRollingResistance_BP() const;

	double GetParticleTerrainRollingResistance() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Terrain Surface Friction"))
	void SetParticleTerrainSurfaceFriction_BP(float ParticleTerrainSurfaceFriction);

	void SetParticleTerrainSurfaceFriction(double ParticleTerrainSurfaceFriction);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Terrain Surface Friction"))
	float GetParticleTerrainSurfaceFriction_BP() const;

	double GetParticleTerrainSurfaceFriction() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Terrain Youngs Modulus"))
	void SetParticleTerrainYoungsModulus_BP(float ParticleTerrainYoungsModulus);

	void SetParticleTerrainYoungsModulus(double ParticleTerrainYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Terrain Youngs Modulus"))
	float GetParticleTerrainYoungsModulus_BP() const;

	double GetParticleTerrainYoungsModulus() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Particle",
		Meta = (DisplayName = "Set Particle Youngs Modulus"))
	void SetParticleYoungsModulus_BP(float ParticleYoungsModulus);

	void SetParticleYoungsModulus(double ParticleYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Compaction",
		Meta = (DisplayName = "Get Particle Youngs Modulus"))
	float GetParticleYoungsModulus_BP() const;

	double GetParticleYoungsModulus() const;

	// Excavation contact properties.
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_TerrainExcavationContactProperties TerrainExcavationContact;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Aggregate Stiffness Multiplier"))
	void SetAggregateStiffnessMultiplier_BP(float AggregateStiffnessMultiplier);

	void SetAggregateStiffnessMultiplier(double AggregateStiffnessMultiplier);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Aggregate Stiffness Multiplier"))
	float GetAggregateStiffnessMultiplier_BP() const;

	double GetAggregateStiffnessMultiplier() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Excavation Stiffness Multiplier"))
	void SetExcavationStiffnessMultiplier_BP(float ExcavationStiffnessMultiplier);

	void SetExcavationStiffnessMultiplier(double ExcavationStiffnessMultiplier);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Excavation Stiffness Multiplier"))
	float GetExcavationStiffnessMultiplier_BP() const;

	double GetExcavationStiffnessMultiplier() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Depth Decay Factor"))
	void SetDepthDecayFactor_BP(float DepthDecayFactor);

	void SetDepthDecayFactor(double DepthDecayFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Depth Decay Factor"))
	float GetDepthDecayFactor_BP() const;

	double GetDepthDecayFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Depth Increase Factor"))
	void SetDepthIncreaseFactor_BP(float DepthIncreaseFactor);

	void SetDepthIncreaseFactor(double DepthIncreaseFactor);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Depth Increase Factor"))
	float GetDepthIncreaseFactor_BP() const;

	double GetDepthIncreaseFactor() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Maximum Aggregate Normal Force"))
	void SetMaximumAggregateNormalForce_BP(float MaximumAggregateNormalForce);

	void SetMaximumAggregateNormalForce(double MaximumAggregateNormalForce);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Maximum Aggregate Normal Force"))
	float GetMaximumAggregateNormalForce_BP() const;

	double GetMaximumAggregateNormalForce() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Set Maximum Contact Depth"))
	void SetMaximumContactDepth_BP(float MaximumContactDepth);

	void SetMaximumContactDepth(double MaximumContactDepth);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Terrain Material Excavation Contact",
		Meta = (DisplayName = "Get Maximum Contact Depth"))
	float GetMaximumContactDepth_BP() const;

	double GetMaximumContactDepth() const;

	virtual void Serialize(FArchive& Archive) override;

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
	FTerrainMaterialBarrier TerrainMaterialNativeBarrier;
	FShapeMaterialBarrier ShapeMaterialNativeBarrier;
};
