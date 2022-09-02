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
 * Both the instance and asset objects are of the same type, but the asset holds a reference to
 * its instance (if one has been created) and the instance always holds a reference to the asset
 * it was created from.
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
 *
 * In addition to the Asset/Instance separation there are also multiple types of materials,
 * currently Shape and Terrain. What they all have in common is that a contact material may
 *  be created between any pair of materials, regardless of their types. Wires use Shape Material.
 *
 * Rules of UPROPERTY updates:
 * 1. Updating a UPROPERTY of an Asset from the Details Panel
 *	in Edit: [permanent property write in asset]
 *	in Play: [permanent property write in asset AND permanent property write in instance AND
 *		propagation to AGX Dynamics]
 *
 * 2. Updating a UPROPERTY of an Asset by calling UFUNCTION
 *	in Edit: [permanent property write in asset]
 *	in Play: [permanent property write in instance AND DO NOT update asset property AND
 *		propagation to AGX Dynamics]
 *
 * 3. Updating a UPROPERTY of an Instance from the Details Panel
 *	in Edit: [does not exists yet]
 *	in Play: [permanent property write in asset AND permanent property write in instance AND
 *		propagation to AGX Dynamics]
 *
 * 4. Updating a UPROPERTY of an Instance by calling UFUNCTION
 *	in Edit: [does not exists yet]
 *	in Play: [permanent property write in instance AND DO NOT update asset property AND
 *		propagation to AGX Dynamics]
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
	virtual void SetFrictionEnabled(bool Enabled)
		PURE_VIRTUAL(UAGX_MaterialBase::SetFrictionEnabled, );

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual bool GetFrictionEnabled() const
		PURE_VIRTUAL(UAGX_MaterialBase::GetFrictionEnabled, return false;);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetRoughness(float Roughness) PURE_VIRTUAL(UAGX_MaterialBase::SetRoughness, );

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetRoughness() const PURE_VIRTUAL(UAGX_MaterialBase::GetRoughness, return 0.f;);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetSurfaceViscosity(float Viscosity)
		PURE_VIRTUAL(UAGX_MaterialBase::SetSurfaceViscosity, );

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetSurfaceViscosity() const
		PURE_VIRTUAL(UAGX_MaterialBase::GetSurfaceViscosity, return 0.f;);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual void SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
		PURE_VIRTUAL(UAGX_MaterialBase::SetAdhesion, );

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetAdhesiveForce() const
		PURE_VIRTUAL(UAGX_MaterialBase::GetAdhesiveForce, return 0.f;);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	virtual float GetAdhesiveOverlap() const
		PURE_VIRTUAL(UAGX_MaterialBase::GetAdhesiveOverlap, return 0.f;);

	/**
	 * Copies all properties (even properties set during Play) to the asset such
	 * that the data is saved permanently in the asset, even after Play.
	 * 
	 * If this function is called on an instance, the properties are copied
	 * from the instance to the asset it was created from, permanently.
	 * If this function is called on an asset, the properties are copied from
	 * its instance (if it exists) to itself such that the data is saved permanently.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Material")
	virtual void CommitToAsset() PURE_VIRTUAL(UAGX_MaterialBase::CommitToAsset, );

public:
	virtual ~UAGX_MaterialBase() = default;

	/**
	 * If PlayingWorld is an in-game World and this material is a UAGX_ShapeMaterial or
	 * UAGX_TerrainMaterial, returns an instance or representing the material asset throughout the
	 * lifetime of the GameInstance. If this is already an instance, it returns itself. Returns
	 * null if not in-game (invalid call).
	 */
	virtual UAGX_MaterialBase* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateInstance, return nullptr;);

	virtual FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_MaterialBase::GetOrCreateShapeMaterialNative, return nullptr;);

protected:
	void CopyShapeMaterialProperties(const UAGX_MaterialBase* Source);

	// The AGX postfix in these function names are due to name collisions with UE-functions.
	virtual bool IsAssetAGX() const PURE_VIRTUAL(UAGX_MaterialBase::IsAsset, return false;);
	virtual bool IsInstanceAGX() const PURE_VIRTUAL(UAGX_MaterialBase::IsInstance, return false;);
};
