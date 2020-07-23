#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_MotionControl.h"
#include "RigidBodyBarrier.h"

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

	/// The mass of the body.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	float Mass;

	/// The three-component diagonal of the inertia tensor.
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Dynamics")
	FVector InertiaTensorDiagonal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics")
	FVector AngularVelocity;

	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Dynamics")
	TEnumAsByte<enum EAGX_MotionControl> MotionControl;

	/**
	 * Write transformations from AGX Dynamics to the Actor's Root Component. Only allowed when the
	 * owning actor has a single AGX Rigid Body Component.
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Dynamics")
	uint8 bTransformRootComponent : 1;

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
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	void DisableTransformRootCompIfMultiple();
#endif

private:
	// The AGX Dynamics object only exists while simulating. Initialized in
	// BeginPlay and released in EndPlay.
	FRigidBodyBarrier NativeBarrier;
};
