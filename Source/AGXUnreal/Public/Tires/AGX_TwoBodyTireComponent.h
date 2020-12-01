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

	// TODO BEFORE MERGE: add comments

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialStiffness = 500000.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralStiffness = 50000000.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingStiffness = 250000.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalStiffness = 250000.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float RadialDamping = 16666.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float LateralDamping = 1666666.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float BendingDamping = 8333.f;

	UPROPERTY(EditAnywhere, Category = "AGX Tire Dynamics")
	float TorsionalDamping = 8333.f;

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

	void CopyFrom(const FTwoBodyTireBarrier& Barrier);

protected:
	virtual void AllocateNative() override;

	virtual void UpdateNativeProperties() override;

private:
	FTwoBodyTireBarrier* CreateTwoBodyTireBarrier();
};
