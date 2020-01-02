#pragma once

#include <tuple>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_LogCategory.h"
#include "AGX_CollisionGroups.generated.h"


UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CollisionGroups : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UAGX_CollisionGroups();

	// Adds all collision groups to all child shape components. Useful
	// when e.g. adding a new shape to a rigid body.
	void ForceRefreshChildShapes();

	void Serialize(FArchive& Ar) override;

#if WITH_EDITOR

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

	/**
	 * List of collision groups.
	 * Note: Changing this property will affect all AGX_ShapeComponent belonging
	 * to the actor that is parent to this object.
	 */
	UPROPERTY(EditAnywhere, BluePrintReadOnly, Category = "AGX Collision Groups")
	TArray<FName> CollisionGroups;

private:
	void ApplyCollisionGroupChanges(FPropertyChangedEvent& PropertyChangedEvent);

private:
	TArray<FName> CollisionGroupsLastChange;
};
