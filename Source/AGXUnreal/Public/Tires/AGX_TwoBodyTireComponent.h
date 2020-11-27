#pragma once

// AGXUnreal includes.
#include "Tire/AGX_TireComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TwoBodyTireComponent.generated.h"

/**
 * TODO add descr.
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TwoBodyTireComponent : public UAGX_TireComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference HubRigidBody;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference TireRigidBody;

public:
	UAGX_TwoBodyTireComponent() = default;
	virtual ~UAGX_TwoBodyTireComponent() = default;
};
