#pragma once

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TireComponent.h"
#include "AGX_RigidBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TwoBodyTireComponent.generated.h"

class FTwoBodyTireBarrier;

/**
 * TwoBodyTire is a tire model that uses two Rigid Bodies, a tire Rigid Body and a hub Rigid Body.
 * The axis of rotation is always assumed to be around the y-axis of the final tire transform which
 * is defined by the tire Rigid Body's transform plus a local offset given by LocalLocation and
 * LocalRotation.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TwoBodyTireComponent : public UAGX_TireComponent
{
	GENERATED_BODY()

public:
	UAGX_TwoBodyTireComponent();
	virtual ~UAGX_TwoBodyTireComponent() = default;

	/**
	 * Outer radius of the tire.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float OuterRadius;

	/**
	 * Inner radius of the tire (and outer radius of hub).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float InnerRadius;

	/**
	 * Reference to the Tire Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference TireRigidBody;

	/**
	 * Reference to the Hub Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference HubRigidBody;

	/**
	 * Tire relative location from Tire Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FVector LocalLocation;

	/**
	 * Tire relative rotation from Tire Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FRotator LocalRotation;

	/**
	 * Radial stiffness affects translation orthogonal to tire rotation axis.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialStiffness = 500000.f;

	/**
	 * Lateral stiffness affects translation in axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralStiffness = 50000000.f;

	/**
	 * Bending stiffness affects rotation orthogonal to axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingStiffness = 250000.f;

	/**
	 * Torsional stiffness affects rotation in axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalStiffness = 250000.f;

	/**
	 * Radial damping affects translation orthogonal to tire rotation axis.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialDamping = 16666.f;

	/**
	 * Lateral damping affects translation in axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralDamping = 1666666.f;

	/**
	 * Bending damping affects rotation orthogonal to axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingDamping = 8333.f;

	/**
	 * Torsional damping affects rotation in axis of rotation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalDamping = 8333.f;

	/**
	 * Set the implicit friction multiplier in order to get different behavior for different
	 * friction directions (forwards, sideways). This is only necessary for implicit contact
	 * materials, since for explicit ones, this can be set directly at the contact material instead.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	FVector2D ImplicitFrictionMultiplier{1.f, 1.f};

	UAGX_RigidBodyComponent* GetHubRigidBody() const;

	UAGX_RigidBodyComponent* GetTireRigidBody() const;

	FTransform GetGlobalTireTransform() const;

	void CopyFrom(const FTwoBodyTireBarrier& Barrier);

	bool IsDefaultSubObjectOfTwoBodyTireActor() const;

protected:
	virtual void AllocateNative() override;

	virtual void UpdateNativeProperties() override;

	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif

private:
	FTwoBodyTireBarrier* CreateTwoBodyTireBarrier();
};
