// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

#include "Materials/AGX_TerrainMaterialBase.h"

#include "AGX_TerrainMaterialAsset.generated.h"

class UAGX_TerrainMaterialInstance;

/**
 * Defines the material for a terrain. Affects both surface and bulk properties.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_TerrainMaterialAsset : public UAGX_TerrainMaterialBase
{
public:
	/// \note Class comment above is used as tooltip in Content Browser etc, so trying to keep it
	/// simple and user-centered, while providing a more programmer-aimed comment below.

	// This class represents terrain material properties as an in-Editor asset that can be
	// serialized to disk.
	//
	// It has no connection with the actual native AGX terrain material. Instead, its sibling class
	// UAGX_TerrainMaterialInstance handles all interaction with the actual native AGX terrain
	// materials.Therefore, all in - game objects with Uproperty terrain material pointers need to
	// swap their pointers to in - game UAGX_TerrainMaterialInstances.

	GENERATED_BODY()

	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;

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
	virtual void SetAdhesionOverlapFactor(float AdhesionOverlapFactor) override;
	virtual float GetAdhesionOverlapFactor() const override;

	virtual void SetCohesion(float Cohesion) override;
	virtual float GetCohesion() const override;

	virtual void SetDensity(float Density) override;
	virtual float GetDensity() const override;

	virtual void SetDilatancyAngle(float DilatancyAngle) override;
	virtual float GetDilatancyAngle() const override;

	virtual void SetFrictionAngle(float FrictionAngle) override;
	virtual float GetFrictionAngle() const override;

	virtual void SetMaxDensity(float MaxDensity) override;
	virtual float GetMaxDensity() const override;

	virtual void SetPoissonsRatio(float PoissonsRatio) override;
	virtual float GetPoissonsRatio() const override;

	virtual void SetSwellFactor(float SwellFactor) override;
	virtual float GetSwellFactor() const override;

	virtual void SetYoungsModulus(float YoungsModulus) override;
	virtual float GetYoungsModulus() const override;

	// Compaction properties.
	virtual void SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate) override;
	virtual float GetAngleOfReposeCompactionRate() const override;

	virtual void SetBankStatePhi(float Phi0) override;
	virtual float GetBankStatePhi() const override;

	virtual void SetCompactionTimeRelaxationConstant(
		float CompactionTimeRelaxationConstant) override;
	virtual float GetCompactionTimeRelaxationConstant() const override;

	virtual void SetCompressionIndex(float CompressionIndex) override;
	virtual float GetCompressionIndex() const override;

	virtual void SetHardeningConstantKe(float K_e) override;
	virtual float GetHardeningConstantKe() const override;

	virtual void SetHardeningConstantNe(float N_e) override;
	virtual float GetHardeningConstantNe() const override;

	virtual void SetPreconsolidationStress(float PreconsolidationStress) override;
	virtual float GetPreconsolidationStress() const override;

	virtual void SetStressCutOffFraction(float StressCutOffFraction) override;
	virtual float GetStressCutOffFraction() const override;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();

	void WriteSurfacePropertyToInstance(const FName& PropertyName);
	void WriteBulkPropertyToInstance(const FName& PropertyName);
	void WriteCompactionPropertyToInstance(const FName& PropertyName);
#endif

	TWeakObjectPtr<UAGX_TerrainMaterialInstance> TerrainMaterialInstance;
};
