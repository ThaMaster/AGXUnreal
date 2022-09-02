// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
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

	virtual void SetRoughness(float Roughness) override;
	virtual float GetRoughness() const override;

	virtual void SetSurfaceViscosity(float Viscosity) override;
	virtual float GetSurfaceViscosity() const override;

	virtual void SetAdhesion(float AdhesiveForce, float AdhesiveOverlap) override;
	virtual float GetAdhesiveForce() const override;
	virtual float GetAdhesiveOverlap() const override;


	// Bulk properties.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetDensity(float InDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetYoungsModulus(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetYoungsModulus() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetBulkViscosity(float InBulkViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetBulkViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetSpookDamping(float InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetSpookDamping() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetMinMaxElasticRestLength(float InMin, float InMax);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetMinElasticRestLength() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	float GetMaxElasticRestLength() const;


	// Wire properties.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetYoungsModulusStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusStretch(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetYoungsModulusBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusBend(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetSpookDampingStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingStretch(float InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	float GetSpookDampingBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingBend(float InSpookDamping);

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
