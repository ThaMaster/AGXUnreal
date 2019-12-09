// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Materials/AGX_ContactMaterialMechanicsApproach.h"
#include "Materials/AGX_ContactMaterialReductionMode.h"
#include "AGX_ContactMaterialBase.generated.h"

class UAGX_ContactMaterialAsset;
class UAGX_ContactMaterialInstance;

/**
 * Defines material properties for contacts between AGX Shapes with specific AGX Materials. This will override many
 * of their individual material properties (does for example not override ones effecting mass, such as density).
 *
 * ContactMaterials are created by the user in-Editor by creating a UAGX_ContactMaterialAsset. In-Editor they are
 * treated as assets and can be referenced by the Material Manager..
 *
 * When game begins playing, one UAGX_ContactMaterialInstance will be created for each UAGX_ContactMaterialAsset that
 * is referenced by the Material Manager. The UAGX_ContactMaterialInstance will create the actual native
 * AGX ContactMaterial and add it to the simulation. The in-game Material Manager that referenced the
 * UAGX_ContactMaterialAsset will swap its reference to the in-game created UAGX_ContactMaterialInstance instead. This
 * means that ultimately only UAGX_ContactMaterialInstances will be referenced in-game. When play stops the in-Editor
 * state will be returned.
 *
 * Note that this means that UAGX_ContactMaterialAssets that are not referenced the Material Manager will be inactive.

 * Note also that it is not allowed to replace the Materials properties after instance has been created.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", abstract, AutoExpandCategories = ("ContactMaterial Properties"))
class AGXUNREAL_API UAGX_ContactMaterialBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * First material.
	 */
	UPROPERTY(EditAnywhere, Category = "Materials")
	UAGX_MaterialBase* Material1;

	/**
	 * Second material. Can be same as first material.
	 */
	UPROPERTY(EditAnywhere, Category = "Materials")
	UAGX_MaterialBase* Material2;

	/**
	 * Solvers to use to calculate the normal and friction equations when objects with this contact material collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	EAGX_ContactSolver ContactSolver;

	/**
	 * Whether contact reduction should be enabled and to what extent.
	 *
	 * By using contact reduction, the number of contact points later submitted to the solver as contact constraint
	 * can be heavily reduced, hence improving performance.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialReductionMode ContactReduction;

	/**
	 * AGX use by default a contact point based method for calculating the corresponding response between two
	 * overlapping geometries.
	 *
	 * There is also a different method available which is named area contact approach. This method will try to
	 * calculate the spanning area between the overlapping contact point. This can result in a better approximation
	 * of the actual overlapping volume and the stiffness in the response (contact constraint).
	 *
	 * In general, this will lead to slightly less stiff, more realistic contacts and therefore the Young’s modulus
	 * value usually has to be increased to get a similar simulation results as running the simulation without the
	 * contact area approach.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialMechanicsApproach MechanicsApproach;

	/**
	 * The friction model used when two objects with this contact material collides.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	EAGX_FrictionModel FrictionModel;

	/**
	 * Whether surface friction should be calculated in the solver for this Contact Material.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	bool bSurfaceFrictionEnabled;

	/**
	 * Friction in all directions if 'Secondary Friction Coefficient' is disable, else only in the primary direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double FrictionCoefficient;

	/*
	 * Friction in the secondary direction, if enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction",
		Meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bUseSecondaryFrictionCoefficient"))
	double SecondaryFrictionCoefficient;

	/**
	 * Whether it should be possible to define friction coefficient per each of the two perpendicular surface direction.
	 *
	 * If enabled, 'Friction Coefficient' represents the primary direction and 'Secondary Friction Coefficient'
	 * represents the secondary direction.
	 *
	 * If disable, 'Friction Coefficient' represents all directions and 'Secondary Friction Coefficient' is not used.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondaryFrictionCoefficient;

	/**
	 * Surface viscosity, telling how 'wet' the friction is between the colliding materials.
	 *
	 * Represents all surface directions if 'Secondary Surface Viscosity' is disable, else only in the primary
	 * direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double SurfaceViscosity;

	/**
	 * Surface viscosity in the secondary direction, if enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction",
		Meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bUseSecondarySurfaceViscosity"))
	double SecondarySurfaceViscosity;

	/**
	 * Whether it should be possible to define surface viscosity per each of the two perpendicular surface direction.
	 *
	 * If enabled, 'Surface Viscosity' represents the primary direction and 'Secondary Surface Viscosity' represents
	 * the secondary direction.
	 *
	 * If disable, 'Surface Viscosity' represents all directions and 'Secondary Surface Viscosity' is not used.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondarySurfaceViscosity;

	/**
	 * Material restitution, i.e. how "bouncy" the normal collisions are.
	 *
	 * A value of 1.0 means that the body does not lose energy during normal-collisions.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Restitution;

	/**
	 * Young's modulus of the contact material. Same as spring coefficient k.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double YoungsModulus;

	/**
	 * Damping factor which represents the time the contact constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double Damping;

	/**
	 * The attractive force between two colliding objects, in Netwon.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double AdhesiveForce;

	/**
	 * Allowed overlap from surface for resting contact, in meters.
	 *
	 * At lower overlap, the adhesion force will take effect.
	 * At this overlap, no adhesive force is applied.
	 * At higher overlap, the (usual) contact force is applied.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	double AdhesiveOverlap;

public:
	/**
	 * Invokes the member function GetOrCreateInstance() on ContactMaterial pointed to by Property, assigns the return
	 * value to Property, and then returns it. Returns null and does nothing if PlayingWorld is not an in-game world.
	 */
	static UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld, UAGX_ContactMaterialBase*& Property);

public:
	UAGX_ContactMaterialBase();

	virtual ~UAGX_ContactMaterialBase();

	/**
	 * If PlayingWorld is an in-game World and this ContactMaterial is a UAGX_ContactMaterialAsset, returns a
	 * UAGX_ContactMaterialInstance representing the ContactMaterial asset throughout the lifetime of the GameInstance.
	 * If this is already a UAGX_ContactMaterialInstance it returns itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_ContactMaterialBase::GetOrCreateInstance, return nullptr;);

	/**
	 * If this ContactMaterial is a UAGX_ContactMaterialInstance, returns the UAGX_ContactMaterialAsset it was created
	 * from (if it still exists). Else returns null.
	 */
	virtual UAGX_ContactMaterialAsset* GetAsset()
		PURE_VIRTUAL(UAGX_ContactMaterialBase::GetOrCreateInstance, return nullptr;);

	void CopyProperties(const UAGX_ContactMaterialBase* Source);
};
