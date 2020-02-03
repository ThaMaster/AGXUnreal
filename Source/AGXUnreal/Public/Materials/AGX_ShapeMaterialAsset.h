// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ShapeMaterialBase.h"
#include "AGX_ShapeMaterialAsset.generated.h"

class UAGX_ShapeMaterialInstance;

/**
 * Defines contacts properties between AGX Shapes as well as properties affecting the mass
 * distribution of AGX Rigid Bodies.
 *
 * Since a contact involves two AGX Shapes, the final parameters used as input to the force
 * equations are a fusion of the two shape's AGX Materials. Though, the way the two material's
 * properties are combined might not be desirable in all scenarios. Therefore, there is another AGX
 * asset called AGX Contact Material that provides a well-defined and more detailed defintion over
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
	// It has no connection with the actual native AGX material.Instead, its sibling class
	// UAGX_ShapeMaterialInstance handles all interaction with the actual native AGX
	// material.Therefore, all in - game objects with Uproperty material pointers need to swap their
	// pointers to in - game UAGX_ShapeMaterialInstances using the static function
	// UAGX_MaterialBase::GetOrCreateInstance.

	GENERATED_BODY()

public:
	virtual UAGX_ShapeMaterialInstance* GetOrCreateShapeMaterialInstance(
		UWorld* PlayingWorld) override;

	TWeakObjectPtr<UAGX_ShapeMaterialInstance> Instance;
};
