#pragma once

// AGXUnreal includes.
#include "AGX_RigidBodyEnums.h"
#include "AGX_MotionControl.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#include "AGX_RigidBodyComponent.generated.h"


UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_RigidBodyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_RigidBodyComponent();

	/// Whether the mass should be computed automatically.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	bool bAutomaticMassProperties = true;

	/// The mass of the body.
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics",
		Meta = (EditCondition = "!bAutomaticMassProperties"))
	float Mass;

	/// The three-component diagonal of the inertia tensor.
	UPROPERTY(
		EditAnywhere, BluePrintReadOnly, Category = "AGX Dynamics",
		Meta = (EditCondition = "!bAutomaticMassProperties"))
	FVector PrincipalInertiae;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	FVector Velocity;

	/**
	 * In radians per second and following the Unreal Editor rotation widget so that a positive
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	FVector AngularVelocity;

	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_MotionControl> MotionControl;

	/**
	 * Decide to where transformation updates from AGX Dynamics should be written.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_TransformTarget> TransformTarget;

	/// Get the native AGX Dynamics representation of this rigid body. Create it if necessary.
	FRigidBodyBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this rigid body. May return nullptr.
	FRigidBodyBarrier* GetNative();

	/// Return true if the AGX Dynamics object has been created. False otherwise.
	bool HasNative();

	/**
	 * Should be called whenever properties (excluding transform and shapes) need to be pushed
	 * onto the native in runtime.
	 */
	void WritePropertiesToNative();

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

#if WITH_EDITOR
	void OnComponentView();
	bool TransformRootComponentAllowed() const;
#endif

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	void InitializeNative();

	// Set native's MotionControl and ensure Unreal has corresponding mobility.
	void InitializeMotionControl();

	void ReadTransformFromNative();
	void WriteTransformToNative();

#if WITH_EDITOR
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#else
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	void DisableTransformRootCompIfMultiple();
#endif

private:
	// The AGX Dynamics object only exists while simulating. Initialized in
	// BeginPlay and released in EndPlay.
	FRigidBodyBarrier NativeBarrier;
};
