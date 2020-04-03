#pragma once

// AGXUnreal includes.
#include "AGX_CollisionGroupPair.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_DisabledCollisionGroupsComponent.generated.h"

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_DisabledCollisionGroupsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX Collisions")
	TArray<FAGX_CollisionGroupPair> DisabledCollisionGroupPairs;

protected:
	/// \todo Not sure if this is the callback I want.
	/// Want to make changes
	virtual void PostLoad() override;

	// This is needed even if the PostLoad is included since the (or a new) CollisionGroupManager
	// may have been created since PostLoad was called.
	virtual void BeginPlay() override;
};
