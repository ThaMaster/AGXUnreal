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
	/*
	 * Not sure how to do this. We want the disabled collisions to be registered with the
	 * CollisionGroupManager as soon as possible, but at the latest on BeginPlay. We want the
	 * disabled collisions to be visible in the Unreal Editor while building the scene as soon as
	 * the UAGX_DisabledCollisionGroupsComponent is created so that the user has a change to know
	 * why collisions aren't detected. This is made more complicated by the fact that the
	 * CollisionGroupManager may not exist yet and we're not allowed to created it during PostLoad
	 * because the level is then in the RunningConstructionScript state. Worse, during PostLoad for
	 * the actual Play session the CollisionGroupManager may still be in line to be created so the
	 * UAGX_DisabledCollisionGroupsComponent creates and configures a new one and then we end up
	 * with two CollisionGroupManagers.
	 *
	 * So we want:
	 * - Register disabled pairs with the existing CollisionGroupManager in PostLoad if we are in
	 * Edit mode but not if we are about to enter Play mode.
	 * - Register disabled pairs with the existing CollisionGroupManager, or create one if needed,
	 * in BeginPlay if we are about to start a Play session, but not during edit mode. I think we
	 * always are in PlayMode when we get BeginPlay.
	 * - Register disabled pairs when a CollisionGroupManager is created in the same level.
	 */

	virtual void PostLoad() override;

	virtual void BeginPlay() override;
};
