#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"

#include "AGX_CollisionGroupPair.h"
#include "AGX_CollisionGroups.generated.h"

/**
 * Defines for which collision groups collision should be disabled for the owning level.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_CollisionGroups : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	TArray<FAGX_CollisionGroupPair> DisabledCollisionGroups;

protected:
	virtual void BeginPlay() override;

private:
	void AddCollisionGroupPairsToSimulation();
};
