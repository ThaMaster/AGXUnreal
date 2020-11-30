#pragma once

// AGXUnreal includes.
#include "Tires/AGX_TireComponent.h"
#include "AGX_RigidBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TwoBodyTireComponent.generated.h"

class FTwoBodyTireBarrier;

/**
 * TODO add descr.
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TwoBodyTireComponent : public UAGX_TireComponent
{
	GENERATED_BODY()

public:
	UAGX_TwoBodyTireComponent() = default;
	virtual ~UAGX_TwoBodyTireComponent() = default;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float OuterRadius;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float InnerRadius;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference TireRigidBody;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference HubRigidBody;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FVector LocalLocation;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FRotator LocalRotation;

	// TODO BEFORE MERGE: Set proper initial values (look at agx::Hinge).

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialStiffness;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralStiffness;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingStiffness;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalStiffness;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialDamping;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralDamping;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingDamping;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalDamping;

	/**
	 * Set the implicit friction multiplier in order to get different behavior for different
	 * friction directions (forwards, sideways). This is only necessary for implicit contact
	 * materials, since for explicit ones, this can be set directly at the contact material instead.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	FVector2D ImplicitFrictionMultiplier;

	UAGX_RigidBodyComponent* GetHubRigidBody() const;

	UAGX_RigidBodyComponent* GetTireRigidBody() const;

	FTransform GetGlobalTireTransform() const;

protected:
	virtual void AllocateNative() override;

	virtual void UpdateNativeProperties() override;

private:
	FTwoBodyTireBarrier* CreateTwoBodyTireBarrier();
};
