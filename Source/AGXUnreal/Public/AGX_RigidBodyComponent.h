#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "RigidBodyBarrier.h"

#include "AGX_RigidBodyComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_RigidBodyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAGX_RigidBodyComponent();

	/// The mass of the body.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Mass;

	/// The three-component diagonal of the inertia tensor.
	UPROPERTY(EditAnywhere, BluePrintReadOnly)
	FVector InertiaTensorDiagonal;

	/// Get the native AGX Dynamics representation of this rigid body. Create it if necessary.
	FRigidBodyBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this rigid body. May return nullptr.
	FRigidBodyBarrier* GetNative();

	/// Return true if the AGX Dynamics object has been created. False otherwise.
	bool HasNative();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	void InitializeNative();

private:
	FRigidBodyBarrier NativeBarrier;
};
