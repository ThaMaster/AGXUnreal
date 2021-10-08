#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ShapeMaterialBase.h"
#include "Materials/ShapeMaterialBarrier.h" /// \todo Shouldn't be necessary here since we have a destructor in cpp file!

#include "AGX_ShapeMaterialInstance.generated.h"

class UAGX_ShapeMaterialAsset;

/**
 * Represents a native AGX material in-game. Should never exist when not playing.
 *
 * Should only ever be created using the static function CreateFromAsset, copying data from its
 * sibling class UAGX_ShapeMaterialAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_ShapeMaterialInstance : public UAGX_ShapeMaterialBase
{
	GENERATED_BODY()

public:

	// Bulk properties.
	virtual void SetDensity(float InDensity) override;
	virtual float GetDensity() const override;

	virtual void SetYoungsModulus(float InYoungsModulus) override;
	virtual float GetYoungsModulus() const override;

	virtual void SetBulkViscosity(float InBulkViscosity) override;
	virtual float GetBulkViscosity() const override;

	virtual void SetSpookDamping(float InSpookDamping) override;
	virtual float GetSpookDamping() const override;

	virtual void SetMinMaxElasticRestLength(float InMin, float InMax) override;
	virtual float GetMinElasticRestLength() const override;
	virtual float GetMaxElasticRestLength() const override;

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

	// Wire properties.
	virtual float GetYoungsModulusStretch() const;
	virtual void SetYoungsModulusStretch(float InYoungsModulus);

	virtual float GetYoungsModulusBend() const;
	virtual void SetYoungsModulusBend(float InYoungsModulus);

	virtual float GetSpookDampingStretch() const;
	virtual void SetSpookDampingStretch(float InSpookDamping);

	virtual float GetSpookDampingBend() const;
	virtual void SetSpookDampingBend(float InSpookDamping);

	static UAGX_ShapeMaterialInstance* CreateFromAsset(UWorld* PlayingWorld, UAGX_ShapeMaterialAsset* Source);

	virtual ~UAGX_ShapeMaterialInstance();

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;

	FShapeMaterialBarrier* GetNative();

	bool HasNative() const;

	void UpdateNativeProperties();

	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;

private:
	// Creates the native AGX material and adds it to the simulation.
	void CreateNative(UWorld* PlayingWorld);

	TUniquePtr<FShapeMaterialBarrier> NativeBarrier;
};
