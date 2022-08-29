// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterialBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialAsset.generated.h"

class UAGX_ShapeMaterialInstance;

/**
 * Defines contacts properties between AGX Shapes as well as properties affecting the mass
 * distribution of AGX Rigid Bodies.
 *
 * Since a contact involves two AGX Shapes, the final parameters used as input to the force
 * equations are a fusion of the two shape's AGX Materials. Though, the way the two material's
 * properties are combined might not be desirable in all scenarios. Therefore, there is another AGX
 * asset called AGX Contact Material that provides a well-defined and more detailed definition over
 * the parameters to use when two specific AGX Materials come in contact with each other.
 *
 * It is preferred to use AGX Contact Materials for superior simulation results!
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeMaterialAsset : public UAGX_ShapeMaterialBase
{
	/// \note Class comment above is used as tooltip in Content Browser etc, so trying to keep it
	/// simple and user-centered, while providing a more programmer-aimed comment below.

	// This class represents material properties as an in-Editor asset that can be serialized to
	// disk.
	//
	// It has no connection with the actual native AGX material. Instead, its sibling class
	// UAGX_ShapeMaterialInstance handles all interaction with the actual native AGX
	// material. Therefore, all in-game objects with Uproperty material pointers need to swap their
	// pointers to in-game UAGX_ShapeMaterialInstances. For more details, see the comment in
	// AGX_MaterialBase.h.

	GENERATED_BODY()

public:
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;

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

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();

	void SynchronizePropertyChangeWithInstance(
		const FName& MemberPropertyName, const FName& PropertyName);

	void WriteSurfacePropertyToInstance(const FName& PropertyName);
	void WriteBulkPropertyToInstance(const FName& PropertyName);
	void WriteWirePropertyToInstance(const FName& PropertyName);
#endif

	TWeakObjectPtr<UAGX_ShapeMaterialInstance> Instance;
};
