#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AGX_CollisionGroups.generated.h"

UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CollisionGroups : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UAGX_CollisionGroups();

	/**
	 * List of collision groups.
	 * Note: Changing this property will affect all AGX_ShapeComponent belonging
	 * to the actor that is parent to this object.
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Collision Groups")
	TArray<FName> CollisionGroups;
};
