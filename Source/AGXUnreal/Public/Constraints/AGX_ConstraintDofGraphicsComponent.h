// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "AGX_ConstraintDofGraphicsComponent.generated.h"

class AAGX_Constraint;
class FPrimitiveSceneProxy;

/**
 * A component that visualizes the constraint's degrees of freedom directly in the viewport.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintDofGraphicsComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	AAGX_Constraint* Constraint;

	UMaterialInterface* GetFreeTranslationMaterial() const;
	UMaterialInterface* GetFreeRotationMaterial() const;
	UMaterialInterface* GetLockedTranslationMaterial() const;
	UMaterialInterface* GetLockedRotationMaterial() const;

	void OnBecameSelected();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	virtual void GetUsedMaterials(
		TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	FMatrix GetRenderMatrix() const;
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface.

	//~ Begin UActorComponent Interface.
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual bool ShouldCollideWhenPlacing() const
	{
		return true;
	}
	//~ End UActorComponent Interface.

private:
	int32 FreeTranslationMaterialIndex;
	int32 FreeRotationMaterialIndex;
	int32 LockedTranslationMaterialIndex;
	int32 LockedRotationMaterialIndex;
};
