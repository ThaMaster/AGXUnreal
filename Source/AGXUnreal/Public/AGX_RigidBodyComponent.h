#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyEnums.h"
#include "AGX_MotionControl.h"
#include "AGX_NativeOwner.h"
#include "AGX_UpropertyDispatcher.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

class UAGX_ShapeComponent;

#include "AGX_RigidBodyComponent.generated.h"

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_RigidBodyComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_RigidBodyComponent();

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetPosition(const FVector& Position);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetPosition() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetRotation(const FQuat& Rotation);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FQuat GetRotation() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetRotator(const FRotator& Rotator);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynmaics")
	FRotator GetRotator() const;

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	bool bEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	bool GetEnabled() const;

	/// The mass of the body [kg].
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics", Meta = (EditCondition = "!bAutoGenerateMass"))
	float Mass;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetMass(float InMass);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	float GetMass() const;

	/// Whether the mass should be computed automatically.
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	bool bAutoGenerateMass = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetAutoGenerateMass(bool bInAuto);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	bool GetAutoGenerateMass() const;

	/// Center of mass offset [cm].
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		Meta = (EditCondition = "!bAutoGenerateCenterOfMassOffset"))
	FVector CenterOfMassOffset;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetCenterOfMassOffset(const FVector& InCoMOffset);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetCenterOfMassOffset() const;

	/// Whether the center of mass offset should be computed automatically.
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	bool bAutoGenerateCenterOfMassOffset = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetAutoGenerateCenterOfMassOffset(bool bInAuto);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	bool GetAutoGenerateCenterOfMassOffset() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	TArray<UAGX_ShapeComponent*> GetShapes() const;

	/**
	 * Explicitly update mass properties.
	 *
	 * E.g., when the density is changed on a material which this rigid body depends on. This method
	 * uses the current mass property update mask. I.e., if this rigid body has an explicitly
	 * assigned mass, mass will not be updated during this call.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void UpdateMassProperties();

	/**
	 * Calculate the mass of this rigid body using the volume and density of added geometries.
	 *
	 * \note This calculated mass isn't necessary the same as getMassProperties()->getMass() since
	 * the mass in mass properties could be assigned (i.e., explicit).
	 *
	 * @return The total mass of this rigid body, calculated given volume and density of added
	 * geometries.
	 */
	double CalculateMass() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics", Meta = (DisplayName = "CalculateMass"))
	float CalculateMass_BP() const;

	/// The three-component diagonal of the inertia tensor.
	UPROPERTY(
		EditAnywhere, Category = "AGX Dynamics",
		Meta = (EditCondition = "!bAutoGeneratePrincipalInertia"))
	FVector PrincipalInertia;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetPrincipalInertia(const FVector& InPrincipalInertia);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetPrincipalInertia() const;

	/// Whether the principal inertia should be computed automatically.
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	bool bAutoGeneratePrincipalInertia = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetAutoGeneratePrincipalInertia(bool bInAuto);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	bool GetAutoGeneratePrincipalInertia() const;

	/// The velocity of the body [cm/s].
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	FVector Velocity;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetVelocity(const FVector& InVelocity);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetVelocity() const;

	/**
	 * In degrees per second and following the Unreal Editor rotation widget so that a positive
	 * angular velocity about some axis produces increasing rotation values in the editor widget.
	 *
	 * A positive X angular velocity rotates the Z axis towards the Y axis, i.e., roll right. A
	 * right-handed rotation.
	 *
	 * A positive Y angular velocity rotates the X axis towards the Z axis, i.e., pitch up. A
	 * right-handed rotation.
	 *
	 * A positive Z angular velocity rotates the X axis towards the Y axis, i.e, yaw right. A
	 * left-handed rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	FVector AngularVelocity;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetAngularVelocity(const FVector& InAngularVelocity);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetAngularVelocity() const;

	UPROPERTY(EditAnywhere, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_MotionControl> MotionControl;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetMotionControl(TEnumAsByte<enum EAGX_MotionControl> InMotionControl);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_MotionControl> GetMotionControl() const;

	/**
	 * Decide to where transformation updates from AGX Dynamics should be written.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_TransformTarget> TransformTarget;

	/**
	 * Add an external force, given in the world coordinate frame, that will be affecting this body
	 * in the next solve. The force will be applied at the center of mass.
	 *
	 * This member function may only be called when a native AGX Dynamics representation has already
	 * been created, i.e., HasNative() returns true;
	 *
	 * @param Force The force to add, given in the world coordinate frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void AddForceAtCenterOfMass(const FVector& Force);

	/**
	 * Add an external force, given in the world coordinate frame, applied at a point specified in
	 * the world coordinate frame, that will be affecting this body in the next solve. If the
	 * location is different from the center of mass then a torque will be calculated and added as
	 * well.
	 *
	 * This member function may only be called when a native AGX Dynamics representation has already
	 * been created, i.e., HasNative() returns true;
	 *
	 * @param Force The force to add, given in the world coordinate frame.
	 * @param Location The location in the world coordinate frame where the force should be applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void AddForceAtWorldLocation(const FVector& Force, const FVector& Location);

	/**
	 * Add an external force, given in the world coordinate frame, applied at a point specified in
	 * the local coordinate frame, that will be affecting this body in the next solve. If the
	 * location is different from the center of mass then a torque will be calculated and added as
	 * well.
	 *
	 * This member function may only be called when a native AGX Dynamics representation has already
	 * been created, i.e., HasNative() returns true;
	 *
	 * @param Force The force to add, given in world coordinate frame.
	 * @param Location The location in the local coordinate frame where the force should be applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void AddForceAtLocalLocation(const FVector& Force, const FVector& Location);

	/**
	 * Get the external forces accumulated so far by the AddForce member functions, to be applied
	 * to this body in the next solve.
	 * @return The external forces for the next solve accumulated so far.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	FVector GetForce() const;

	/**
	 * Adds an external torque, given in center of mass coordinate frame, that will be affecting
	 * this body in the next solve [Nm].
	 *
	 * @param Torque The torque to add, given in center of mass coordinate frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void AddTorqueLocal(const FVector& Torque);

	/**
	 * Add an external torque, given in the world coordinate frame, that will be affecting this body
	 * in the next solve [Nm].
	 *
	 * @param Torque The torque to add, given in world coordinate frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void AddTorqueWorld(const FVector& Torque);

	/**
	 * Get the external torques accumulated so far by the AddTorque member functions, to be applied
	 * to this body in the next solve.
	 *
	 * @return The external torques for the next solve accumulated so far.
	 */
	FVector GetTorque() const;

	/// Get the native AGX Dynamics representation of this rigid body. Create it if necessary.
	FRigidBodyBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this rigid body. May return nullptr.
	FRigidBodyBarrier* GetNative();

	const FRigidBodyBarrier* GetNative() const;

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.

	/**
	 * Copy direct rigid body properties from the barrier to this component.
	 *
	 * This includes things like mass, velocity, and motion control. Does not copy sub-component
	 * data such as shapes or materials.
	 *
	 * @param Barrier The AGX Dynamics RigidBody to copy from.
	 */
	void CopyFrom(const FRigidBodyBarrier& Barrier);

	static TArray<UAGX_RigidBodyComponent*> GetFromActor(const AActor* Actor);
	static UAGX_RigidBodyComponent* GetFirstFromActor(const AActor* Actor);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

	// ~Begin USceneComponent interface.
#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
#endif
	// ~End USceneComponent interface.

#if WITH_EDITOR
	void OnComponentView();
	bool TransformRootComponentAllowed() const;
#endif

private:
#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every UPROPERTY in PostEditChangeProperty and PostEditChangeChainProperty.
	void InitPropertyDispatcher();
#endif

	// Create the native AGX Dynamics object.
	void InitializeNative();

	// Set native's MotionControl and ensure Unreal has corresponding mobility.
	void InitializeMotionControl();

	/**
	 * Should be called whenever properties (excluding transform and shapes) need to be pushed
	 * onto the native in runtime.
	 */
	void WritePropertiesToNative();

	void ReadTransformFromNative();
	void WriteTransformToNative();

	/// A variant of WriteTransformToNative that only writes if we have a Native to write to.
	void TryWriteTransformToNative();

#if WITH_EDITOR
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#else
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	void DisableTransformRootCompIfMultiple();
#endif

private:
#if WITH_EDITORONLY_DATA
	FAGX_UpropertyDispatcher<UAGX_RigidBodyComponent> PropertyDispatcher;
#endif

	// The AGX Dynamics object only exists while simulating. Initialized in
	// BeginPlay and released in EndPlay.
	FRigidBodyBarrier NativeBarrier;
};
