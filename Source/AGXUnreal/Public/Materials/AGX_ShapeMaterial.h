// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_MaterialBase.h"
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterial.generated.h"


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
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_ShapeMaterial : public UAGX_MaterialBase
{
	GENERATED_BODY()

public:
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

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetDensity_BP(float InDensity);

	void SetDensity(FAGX_Real InDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetDensity_BP() const;

	FAGX_Real GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetYoungsModulus_BP(float InYoungsModulus);

	void SetYoungsModulus(FAGX_Real InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetYoungsModulus_BP() const;

	FAGX_Real GetYoungsModulus() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetBulkViscosity_BP(float InBulkViscosity);

	void SetBulkViscosity(FAGX_Real InBulkViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetBulkViscosity_BP() const;

	FAGX_Real GetBulkViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetSpookDamping_BP(float InSpookDamping);

	void SetSpookDamping(FAGX_Real InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetSpookDamping_BP() const;

	FAGX_Real GetSpookDamping() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetMinMaxElasticRestLength_BP(float InMin, float InMax);

	void SetMinMaxElasticRestLength(FAGX_Real InMin, FAGX_Real InMax);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetMinElasticRestLength_BP() const;

	FAGX_Real GetMinElasticRestLength() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetMaxElasticRestLength_BP() const;

	FAGX_Real GetMaxElasticRestLength() const;


	// Wire properties.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetYoungsModulusStretch_BP() const;

	FAGX_Real GetYoungsModulusStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusStretch_BP(float InYoungsModulus);

	void SetYoungsModulusStretch(FAGX_Real InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetYoungsModulusBend_BP() const;

	FAGX_Real GetYoungsModulusBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusBend_BP(float InYoungsModulus);

	void SetYoungsModulusBend(FAGX_Real InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetSpookDampingStretch_BP() const;

	FAGX_Real GetSpookDampingStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingStretch_BP(float InSpookDamping);

	void SetSpookDampingStretch(FAGX_Real InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetSpookDampingBend_BP() const;

	FAGX_Real GetSpookDampingBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingBend_BP(float InSpookDamping);

	void SetSpookDampingBend(FAGX_Real InSpookDamping);

	virtual void CommitToAsset() override;

	void CopyFrom(const FShapeMaterialBarrier* Source);

	static UAGX_ShapeMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeMaterial* Source);

	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld) override;
	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld) override;

	FShapeMaterialBarrier* GetNative();
	bool HasNative() const;
	void UpdateNativeProperties();	

	bool IsInstance() const override;

private:	
	void CreateNative(UWorld* PlayingWorld);

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

	TWeakObjectPtr<UAGX_ShapeMaterial> Asset;
	TWeakObjectPtr<UAGX_ShapeMaterial> Instance;
	TUniquePtr<FShapeMaterialBarrier> NativeBarrier;
};
