// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterialBulkProperties.h"
#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"
#include "Materials/AGX_ShapeMaterialWireProperties.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_MaterialBase.generated.h"

class FShapeMaterialBarrier;

/**
 * Defines physical properties of AGX Shapes, AGX Terrains, AGX Wires, etc.
 *
 * Materials can exist in two different contexts: editing and play.
 *
 * Editing is the regular Unreal Editor mode, where we use the editor to set properties and assign
 * assets. In this case the material is an asset. New materials are created and existing materials
 * are edited in the Content Browser. Material properties, such as on an UAGX_ShapeComponent, are
 * pointers to these assets.
 *
 * During play things are a bit different. This is because we don't want changes made during play to
 * affect the on-disk assets. We therefore decouple edit-mode assets from play session objects by
 * creating clones of the edit mode assets for use during the duration of the play session. The
 * Instances are the only classes that has a native AGX Dynamics object associated with it.
 *
 * Subclasses are used to differentiate between the two types, with the Asset suffix being used for
 * editing mode objects and the Instance suffix being used for the play mode objects. The switch is
 * done in BeginPlay of the class that has the UPROPERTY. A short illustrative example:
 *
 *	UCLASS()
 *	class UMyClass : public UObject
 *	{
 *		GENERATED_BODY()
 *
 *		UPROPERTY()
 *		UAGX_MaterialBase* MyMaterial; // Asset while in editor, Instance during gameplay.
 *
 *		virtual void BeginPlay();
 * 	};
 *
 * 	void UMyClass::BeginPlay()
 * 	{
 *		if (GetWorld()->IsGameWorld())
 *		{
 *			// We are being created in game mode, so get the non-Asset
 *			// instance and swap out the Asset pointer.
 *			UAGX_MaterialInstance* MaterialInstance =
 *				Cast<UAGX_MaterialInstance>(MyMaterial->GetOrCreateInstance());
 *			MyMaterial = Instance;
 *		}
 * 	}
 * There may be a bit more to it, depending on the type of material (see next paragraph) but
 * something like that.
 *
 * In addition to the Asset/Instance separation there are also multiple types of materials,
 * currently Shape and Terrain, each with their own Base, Asset, and Instance classes. What they all
 * have in common is that a contact material may be created between any pair of materials,
 * regardless of their types. Wires use Shape Material.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", abstract, AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_MaterialBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_ShapeMaterialBulkProperties Bulk;

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_ShapeMaterialSurfaceProperties Surface;

	UPROPERTY(EditAnywhere, Category = "Material Properties")
	FAGX_ShapeMaterialWireProperties Wire;

	// Setter and getter for surface properties. These can be here since they are shared by both
	// Shape Materials and Terrain Materials. The Bulk properties differ between Shape and Terrain
	// Materials so each material type have their own set of getters and setters for those. The
	// Wire properties are Shape Material only, so they are in the Shape Material.

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetFrictionEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual bool GetFrictionEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetRoughness(float Roughness);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetRoughness() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetSurfaceViscosity(float Viscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetSurfaceViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetAdhesion(float AdhesiveForce, float AdhesiveOverlap);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetAdhesiveForce() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetAdhesiveOverlap() const;

public:
	virtual ~UAGX_MaterialBase();

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_ShapeMaterialAsset or
	 * UAGX_TerrainMaterialAsset, returns a UAGX_ShapeMaterialInstance or
	 * UAGX_TerrainMaterialInstance representing the material asset throughout the lifetime of the
	 * GameInstance. If this is already a UAGX_ShapeMaterialInstance or a
	 * UAGX_TerrainMaterialInstance, it returns itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateInstance, return nullptr;);

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateShapeMaterialNative, return nullptr;);

protected:
	void CopyShapeMaterialProperties(const UAGX_MaterialBase* Source);
};
