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
	void UpdateNativeProperties();

	static UAGX_RigidBodyComponent* GetFromActor(const AActor* Actor);

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

	void UpdateActorTransformsFromNative();
	void UpdateNativeTransformsFromActor();

private:
	FRigidBodyBarrier NativeBarrier;
};
