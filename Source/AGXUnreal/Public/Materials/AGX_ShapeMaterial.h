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

	virtual void SetFrictionEnabled(bool Enabled) override;
	virtual bool GetFrictionEnabled() const override;

	virtual void SetRoughness(double Roughness) override;
	virtual double GetRoughness() const override;

	virtual void SetRoughness_BP(float Roughness) override;
	virtual float GetRoughness_BP() const override;

	virtual void SetSurfaceViscosity(double Viscosity) override;
	virtual double GetSurfaceViscosity() const override;

	virtual void SetSurfaceViscosity_BP(float Viscosity) override;
	virtual float GetSurfaceViscosity_BP() const override;

	virtual void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap) override;
	virtual double GetAdhesiveForce() const override;
	virtual double GetAdhesiveOverlap() const override;

	virtual void SetAdhesion_BP(float AdhesiveForce, float AdhesiveOverlap) override;
	virtual float GetAdhesiveForce_BP() const override;
	virtual float GetAdhesiveOverlap_BP() const override;

	// Bulk properties.

	void SetDensity(double InDensity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Set Density"))
	void SetDensity_BP(float InDensity);

	double GetDensity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Density"))
	float GetDensity_BP() const;

	void SetYoungsModulus(double InYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Set Youngs Modulus"))
	void SetYoungsModulus_BP(float InYoungsModulus);

	double GetYoungsModulus() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Youngs Modulus"))
	float GetYoungsModulus_BP() const;

	void SetBulkViscosity(double InBulkViscosity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Set Bulk Viscosity"))
	void SetBulkViscosity_BP(float InBulkViscosity);

	double GetBulkViscosity() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Bulk Viscosity"))
	float GetBulkViscosity_BP() const;

	void SetSpookDamping(double InSpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Set Spook Damping"))
	void SetSpookDamping_BP(float InSpookDamping);

	double GetSpookDamping() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Spook Damping"))
	float GetSpookDamping_BP() const;

	void SetMinMaxElasticRestLength(double InMin, double InMax);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Set Min Max Elastic Rest Length"))
	void SetMinMaxElasticRestLength_BP(float InMin, float InMax);

	double GetMinElasticRestLength() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Min Elastic Rest Length"))
	float GetMinElasticRestLength_BP() const;

	double GetMaxElasticRestLength() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Bulk Properties",
		Meta = (DisplayName = "Get Max Elastic Rest Length"))
	float GetMaxElasticRestLength_BP() const;

	// Wire properties.

	double GetYoungsModulusStretch() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Get Youngs Modulus Stretch"))
	float GetYoungsModulusStretch_BP() const;

	void SetYoungsModulusStretch(double InYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Set Youngs Modulus Stretch"))
	void SetYoungsModulusStretch_BP(float InYoungsModulus);

	double GetYoungsModulusBend() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Get Youngs Modulus Bend"))
	float GetYoungsModulusBend_BP() const;

	void SetYoungsModulusBend(double InYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Set Youngs Modulus Bend"))
	void SetYoungsModulusBend_BP(float InYoungsModulus);

	double GetSpookDampingStretch() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Get Spook Damping Stretch"))
	float GetSpookDampingStretch_BP() const;

	void SetSpookDampingStretch(double InSpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Set Spook Damping Stretch"))
	void SetSpookDampingStretch_BP(float InSpookDamping);

	double GetSpookDampingBend() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Get Spook Damping Bend"))
	float GetSpookDampingBend_BP() const;

	void SetSpookDampingBend(double InSpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Material Wire Properties",
		Meta = (DisplayName = "Set Spook Damping Bend"))
	void SetSpookDampingBend_BP(float InSpookDamping);

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
