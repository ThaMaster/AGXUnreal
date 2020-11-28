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
	FAGX_RigidBodyReference HubRigidBody;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float InnerRadius;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference TireRigidBody;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float OuterRadius;

	UAGX_RigidBodyComponent* GetHubRigidBody() const;

	UAGX_RigidBodyComponent* GetTireRigidBody() const;

protected:
	virtual void AllocateNative() override;

	virtual void UpdateNativeProperties() override;

private:
	FTwoBodyTireBarrier* CreateTwoBodyTireBarrier();
};
