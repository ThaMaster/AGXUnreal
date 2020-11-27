#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TireComponent.generated.h"


/**
 * TODO add descr.
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", notplaceable)
class AGXUNREAL_API UAGX_TireComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TireComponent();
	virtual ~UAGX_TireComponent() = default;

	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	float OuterRadius;
};
