#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_TerrainMaterialBase.h"
#include "Materials/TerrainMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"

#include "AGX_TerrainMaterialInstance.generated.h"

class UAGX_TerrainMaterialAsset;

/**
 * Represents a AGX terrain material in-game. Should never exist when not playing.
 *
 * Should only ever be created using the static function CreateFromAsset, copying data from its
 * sibling class UAGX_TerrainMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_TerrainMaterialInstance : public UAGX_TerrainMaterialBase
{
	GENERATED_BODY()

public:
	virtual ~UAGX_TerrainMaterialInstance();

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

	virtual void SetCohesion(float Cohesion);
	virtual float GetCohesion() const;

	virtual void SetDensity(float Density);
	virtual float GetDensity() const;

	virtual void SetDilatancyAngle(float DilatancyAngle);
	virtual float GetDilatancyAngle() const;

	virtual void SetFrictionAngle(float FrictionAngle);
	virtual float GetFrictionAngle() const;

	virtual void SetMaxDensity(float MaxDensity);
	virtual float GetMaxDensity() const;

	virtual void SetPoissonsRatio(float PoissonsRatio);
	virtual float GetPoissonsRatio() const;

	virtual void SetSwellFactor(float SwellFactor);
	virtual float GetSwellFactor() const;

	virtual void SetYoungsModulus(float YoungsModulus);
	virtual float GetYoungsModulus() const;

	// Compaction properties.
	virtual void SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate);
	virtual float GetAngleOfReposeCompactionRate() const;

	virtual void SetBankStatePhi(float Phi0);
	virtual float GetBankStatePhi() const;

	virtual void SetCompactionTimeRelaxationConstant(float CompactionTimeRelaxationConstant);
	virtual float GetCompactionTimeRelaxationConstant() const;

	virtual void SetCompressionIndex(float CompressionIndex);
	virtual float GetCompressionIndex() const;

	virtual void SetHardeningConstantKe(float K_e);
	virtual float GetHardeningConstantKe() const;

	virtual void SetHardeningConstantNe(float N_e);
	virtual float GetHardeningConstantNe() const;

	virtual void SetPreconsolidationStress(float PreconsolidationStress);
	virtual float GetPreconsolidationStress() const;

	virtual void SetStressCutOffFraction(float StressCutOffFraction);
	virtual float GetStressCutOffFraction() const;

	FTerrainMaterialBarrier* GetOrCreateTerrainMaterialNative(UWorld* PlayingWorld);

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;

	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;

	static UAGX_TerrainMaterialInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source);

private:
	// Creates the native AGX terrain material
	void CreateTerrainMaterialNative(UWorld* PlayingWorld);
	void CreateShapeMaterialNative(UWorld* PlayingWorld);

	bool HasTerrainMaterialNative() const;
	bool HasShapeMaterialNative() const;

	FTerrainMaterialBarrier* GetTerrainMaterialNative();
	FShapeMaterialBarrier* GetShapeMaterialNative();

	void UpdateTerrainMaterialNativeProperties();
	void UpdateShapeMaterialNativeProperties();

private:
	TUniquePtr<FTerrainMaterialBarrier> TerrainMaterialNativeBarrier;
	TUniquePtr<FShapeMaterialBarrier> ShapeMaterialNativeBarrier;
};
