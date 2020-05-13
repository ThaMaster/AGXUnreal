#pragma once
#include "Materials/AGX_MaterialBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialBase.generated.h"

class FShapeMaterialBarrier;

/**
 * Represents an AGX shape material.
 *
 * Shape Materials are created by the user in-Editor by creating a UAGX_ShapeMaterialAsset.
 * In-Editor they are treated as assets and can be referenced by either Shapes or Contact Materials.
 *
 * When game begins playing, one UAGX_ShapeMaterialInstance will be created for each
 * UAGX_ShapeMaterialAsset that is referenced by an in-game Shape or Contact Material. The
 * UAGX_ShapeMaterialInstance will create the actual native AGX material and add it to the
 * simulation. The in-game Shape or Contact Material that referenced the UAGX_ShapeMaterialAsset
 * will swap its reference to the in-game created instance instead. This means that ultimately only
 * UAGX_ShapeMaterialInstances will be referenced in-game. When play stops the in-Editor state will
 * be returned.
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
	void CopyFrom(const FShapeMaterialBarrier* Source);
};
