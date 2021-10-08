#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_MaterialBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialBase.generated.h"

class FShapeMaterialBarrier;

/**
 * Represents an AGX shape material.
 *
 * Shape Materials are created by the user in-Editor by creating a UAGX_ShapeMaterialAsset.
 * In-Editor they are treated as assets and can be referenced by Shapes, Wires, and Contact
 * Materials.
 *
 * When game begins playing, one UAGX_ShapeMaterialInstance will be created for each
 * UAGX_ShapeMaterialAsset that is referenced by an in-game Shape, Wire or Contact Material. The
 * UAGX_ShapeMaterialInstance will create the actual native AGX material and add it to the
 * simulation. The in-game Shape, Wire, or Contact Material that referenced the
 * UAGX_ShapeMaterialAsset will swap its reference to the in-game created instance instead. This
 * means that ultimately only UAGX_ShapeMaterialInstances will be referenced in-game. When play
 * stops the in-Editor state will be returned.
 *
 * Note that this also means that UAGX_ShapeMaterialAsset that are not
 * referenced by anything will be inactive.
 *
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_ShapeMaterialBase : public UAGX_MaterialBase
{
	GENERATED_BODY()

public:

	// Setters and getters for bulk properties.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual void SetDensity(float InDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual void SetYoungsModulus(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetYoungsModulus() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual void SetBulkViscosity(float InBulkViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetBulkViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual void SetSpookDamping(float InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetSpookDamping() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual void SetMinMaxElasticRestLength(float InMin, float InMax);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetMinElasticRestLength() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	virtual float GetMaxElasticRestLength() const;

	// Setters and getters for wire properties.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual float GetYoungsModulusStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual void SetYoungsModulusStretch(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual float GetYoungsModulusBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual void SetYoungsModulusBend(float InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual float GetSpookDampingStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual void SetSpookDampingStretch(float InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual float GetSpookDampingBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	virtual void SetSpookDampingBend(float InSpookDamping);

	void CopyFrom(const FShapeMaterialBarrier* Source);
};
