// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ContactMaterialMechanicsApproach.h"
#include "Materials/AGX_ContactMaterialReductionMode.h"
#include "AGX_RigidBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_ContactMaterialBase.generated.h"

class UAGX_ContactMaterialAsset;
class UAGX_ContactMaterialInstance;
class UAGX_ContactMaterialRegistrarComponent;
class FContactMaterialBarrier;
class UAGX_MaterialBase;

/**
 * Defines material properties for contacts between AGX Shapes with specific AGX Materials. This
 * will override many of their individual material properties (does for example not override ones
 * effecting mass, such as density).
 *
 * ContactMaterials are created by the user in-Editor by creating a UAGX_ContactMaterialAsset.
 * In-Editor they are treated as assets and can be referenced by a Contact Material Registrar
 * Component.
 *
 * When game begins playing, one UAGX_ContactMaterialInstance will be created for each
 * UAGX_ContactMaterialAsset that is referenced by a Contact Material Registrar Component. The
 * UAGX_ContactMaterialInstance will create the actual native AGX ContactMaterial and add it to the
 * simulation. The in-game Contact Material Registrar Component that referenced the
 * UAGX_ContactMaterialAsset will swap its reference to the in-game created
 * UAGX_ContactMaterialInstance instead. This means that ultimately only
 * UAGX_ContactMaterialInstances will be referenced in-game. When play stops the in-Editor state
 * will be returned.
 *
 * Note that this means that UAGX_ContactMaterialAssets that are not referenced a Contact Material
 * Registrar Component will be inactive.
 *
 * Note also that it is not allowed to replace the Materials properties after instance has been
 * created.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", abstract,
	AutoExpandCategories = ("ContactMaterial Properties"))
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
	 * Solvers to use to calculate the normal and friction equations when objects with this contact
	 * material collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	EAGX_ContactSolver ContactSolver;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetContactSolver(EAGX_ContactSolver InContactSolver);

	/**
	 * Whether contact reduction should be enabled and to what extent.
	 *
	 * By using contact reduction, the number of contact points later submitted to the solver as
	 * contact constraint can be heavily reduced, hence improving performance.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialReductionMode ContactReduction;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetContactReductionMode(EAGX_ContactReductionMode InReductionMode);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetContactReductionBinResolution(uint8 InBinResolution);

	/**
	 * AGX use by default a contact point based method for calculating the corresponding response
	 * between two overlapping geometries.
	 *
	 * There is also a different method available which is named area contact approach. This method
	 * will try to calculate the spanning area between the overlapping contact point. This can
	 * result in a better approximation of the actual overlapping volume and the stiffness in the
	 * response (contact constraint).
	 *
	 * In general, this will lead to slightly less stiff, more realistic contacts and therefore the
	 * Young’s modulus value usually has to be increased to get a similar simulation results as
	 * running the simulation without the contact area approach.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialMechanicsApproach MechanicsApproach;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetUseContactAreaApproach(bool bInUseContactAreaApproach);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetMinElasticRestLength(float InMinLength);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetMaxElasticRestLength(float InMaxLength);

	/**
	 * The friction model used when two objects with this contact material collides.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	EAGX_FrictionModel FrictionModel;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetFrictionModel(EAGX_FrictionModel InFrictionModel);

	/**
	 * Constant normal force used by the friction model 'Constant Normal Force Box Friction'. [N]
	 *
	 * This should be set to an estimation of the force, in Newtons, by which the two colliding
	 * objects are being pushed together. If the main contributor to this force is gravity then
	 * this value should be set to the mass of the upper object and any additional load it is
	 * carrying times the gravitational acceleration in m/s^2, i.e. around 9.8.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	FAGX_Real NormalForceMagnitude;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetNormalForceMagnitude(float InNormalForceMagnitude);

	/**
	 * Whether the 'Normal Force Magnitude' should be scaled by contact point depth.
	 * Only used by friction model 'Constant Normal Force Box Friction'.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	bool bScaleNormalForceWithDepth;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetScaleNormalForceWithDepth(bool bEnabled);

	/**
	 * Whether surface friction should be calculated in the solver for this Contact Material.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	bool bSurfaceFrictionEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetSurfaceFrictionEnabled(bool bInSurfaceFrictionEnabled);

	/**
	 * Friction in all directions if 'Secondary Friction Coefficient' is disable, else only in the
	 * primary direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real FrictionCoefficient;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetFrictionCoefficient(float InFrictionCoefficient);

	/**
	 * Friction in the secondary direction, if enabled.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(ClampMin = "0.0", UIMin = "0.0",
			 // We would like to include a check for oriented friction model here, but Unreal
			 // Engine 4.26 doesn't support that in combination with InlineEditConditionToggle on
			 // bUseSecondarySurfaceViscosity.
			 EditCondition = "bUseSecondaryFrictionCoefficient"))
	FAGX_Real SecondaryFrictionCoefficient;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetSecondaryFrictionCoefficient(float InSecondaryFrictionCoefficient);

	/**
	 * Whether it should be possible to define friction coefficient per each of the two
	 * perpendicular surface direction.
	 *
	 * If enabled, 'Friction Coefficient' represents the primary direction and 'Secondary Friction
	 * Coefficient' represents the secondary direction.
	 *
	 * If disable, 'Friction Coefficient' represents all directions and 'Secondary Friction
	 * Coefficient' is not used.
	 *
	 * Note that secondary direction friction coefficient is only used by Oriented Friction Models.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondaryFrictionCoefficient;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetUseSecondaryFrictionCoefficient(bool bInUseSecondaryFrictionCoefficient);

	/**
	 * Surface viscosity, telling how 'wet' the friction is between the colliding materials.
	 *
	 * Represents all surface directions if 'Secondary Surface Viscosity' is disable, else only in
	 * the primary direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SurfaceViscosity;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetSurfaceViscosity(float InSurfaceViscosity);

	/**
	 * Surface viscosity in the secondary direction, if enabled.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(ClampMin = "0.0", UIMin = "0.0",
			 // We would like to include a check for oriented friction model here, but Unreal
			 // Engine 4.26 doesn't support that in combination with InlineEditConditionToggle on
			 // bUseSecondarySurfaceViscosity.
			 EditCondition = "bUseSecondarySurfaceViscosity"))
	FAGX_Real SecondarySurfaceViscosity;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetSecondarySurfaceViscosity(float InSecondarySurfaceViscosity);

	/**
	 * Whether it should be possible to define surface viscosity per each of the two perpendicular
	 * surface direction.
	 *
	 * If enabled, 'Surface Viscosity' represents the primary direction and 'Secondary Surface
	 * Viscosity' represents the secondary direction.
	 *
	 * If disable, 'Surface Viscosity' represents all directions and 'Secondary Surface Viscosity'
	 * is not used.
	 *
	 * Note that secondary direction surface viscosity is only used by Oriented Friction Models.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondarySurfaceViscosity;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetUseSecondarySurfaceViscosity(bool bInUseSecondarySurfaceViscosity);

	// clang-format off
	/**
	 * Primary friction/viscosity direction relative to Reference Frame.
	 * Secondary direction will be perpendicular to primary direction.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
			"FrictionModel == EAGX_FrictionModel::OrientedBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	FVector PrimaryDirection;
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetPrimaryDirection(const FVector& InPrimaryDirection);

	// clang-format off
	/**
	 * The name of the actor that contains the component to use as reference frame for a
	 * Oriented Friction Model (component specified by the property
	 * OrientedFrictionReferenceFrameComponent).
	 *
	 * If this name is left empty, the reference frame component is supposed to exist
	 * in the same actor as the ContactMaterialRegistrarComponent that owns this contact material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	FName OrientedFrictionReferenceFrameActor;
	// clang-format on

	// clang-format off
	/**
	 * The component whose transform should be used as Reference Frame for a Oriented Friction
	 * Model.
	 *
	 * The component must be a Rigid Body Component.
	 *
	 * The component must exist in the actor specified by the property
	 * OrientedFrictionReferenceFrameActor, or if the actor is not specified the component is
	 * supposed to exist in the same actor as the ContactMaterialRegistrarComponent that owns this
	 * contact material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedBoxFriction || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	FName OrientedFrictionReferenceFrameComponent;
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool IsOrientedFrictionModel();
	/**
	 * Material restitution, i.e. how "bouncy" the normal collisions are.
	 *
	 * A value of 1.0 means that the body does not lose energy during normal-collisions.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Restitution;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetRestitution(float InRestitution);

	/**
	 * Young's modulus of the contact material. Same as spring coefficient k [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulus;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetYoungsModulus(float InYoungsModulus);

	/**
	 * Spook Damping which represents the time the contact constraint has to fulfill its violation
	 * [s].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SpookDamping;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetSpookDamping(float InSpookDamping);

	/**
	 * The attractive force between two colliding objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveForce;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetAdhesiveForce(float InAdhesiveForce);

	/**
	 * Allowed overlap from surface for resting contact [cm].
	 *
	 * At lower overlap, the adhesion force will take effect.
	 * At this overlap, no adhesive force is applied.
	 * At higher overlap, the (usual) contact force is applied.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveOverlap;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	virtual void SetAdhesiveOverlap(float InAdhesiveOverlap);

public:
	/**
	 * Invokes the member function GetOrCreateInstance() on ContactMaterial pointed to by Property,
	 * assigns the return value to Property, and then returns it. Returns null and does nothing if
	 * PlayingWorld is not an in-game world.
	 */
	static UAGX_ContactMaterialInstance* GetOrCreateInstance(
		UAGX_ContactMaterialRegistrarComponent& Registrar, UAGX_ContactMaterialBase*& Property);

	UAGX_ContactMaterialBase();

	virtual ~UAGX_ContactMaterialBase();

	virtual UAGX_ContactMaterialInstance* GetInstance()
		PURE_VIRTUAL(UAGX_ContactMaterialBase::GetInstance, return nullptr;);

	/**
	 * If PlayingWorld is an in-game World and this ContactMaterial is a UAGX_ContactMaterialAsset,
	 * returns a UAGX_ContactMaterialInstance representing the ContactMaterial asset throughout the
	 * lifetime of the GameInstance. If this is already a UAGX_ContactMaterialInstance it returns
	 * itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_ContactMaterialInstance* GetOrCreateInstance(
		UAGX_ContactMaterialRegistrarComponent& Registrar)
		PURE_VIRTUAL(UAGX_ContactMaterialBase::GetOrCreateInstance, return nullptr;);

	/**
	 * If this ContactMaterial is a UAGX_ContactMaterialInstance, returns the
	 * UAGX_ContactMaterialAsset it was created from (if it still exists). Else returns null.
	 */
	virtual UAGX_ContactMaterialAsset* GetAsset()
		PURE_VIRTUAL(UAGX_ContactMaterialBase::GetAsset, return nullptr;);

	void CopyFrom(const UAGX_ContactMaterialBase* Source);
	void CopyFrom(const FContactMaterialBarrier* Source);
};
