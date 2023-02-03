// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "CollisionGroups/AGX_CollisionGroupPair.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_CollisionGroupDisablerComponent.generated.h"

/**
 * From the Collision Group Disabler Component's detail panel, it is possible to disable
 * collision between AGX Shape Components in the world with a specific collision group
 * associated with them. This Component also exposes functions callable from BluePrint for disabling
 * or re-enabling collision during play.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_CollisionGroupDisablerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_CollisionGroupDisablerComponent();

	/**
	 * A collection of collision group pairs for which collision is disabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Collision Group Pairs")
	TArray<FAGX_CollisionGroupPair> DisabledCollisionGroupPairs;

	/**
	 * Disable collision between two given collision groups.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Collision Group Pairs")
	void DisableCollisionGroupPair(
		const FName& Group1, const FName& Group2, bool HideWarnings = false);

	/**
	 * (Re-)Enable collision between two given collision groups.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Collision Group Pairs")
	void EnableCollisionGroupPair(
		const FName& Group1, const FName& Group2, bool HideWarnings = false);

	void UpdateAvailableCollisionGroups();

	void UpdateAvailableCollisionGroupsFromWorld();

	void RemoveDeprecatedCollisionGroups();

	const TArray<FName>& GetAvailableCollisionGroups() const
	{
		return AvailableCollisionGroups;
	}

	FName& GetSelectedGroup1()
	{
		return SelectedGroup1;
	}

	FName& GetSelectedGroup2()
	{
		return SelectedGroup2;
	}

	bool IsCollisionGroupPairDisabled(
		const FName& CollisionGroup1, const FName& CollisionGroup2) const;

protected:
	virtual void BeginPlay() override;

private:
	void AddCollisionGroupPairsToSimulation();

	bool IsCollisionGroupPairDisabled(
		const FName& CollisionGroup1, const FName& CollisionGroup2, int& OutIndex) const;

	TArray<FName> AvailableCollisionGroups;
	FName SelectedGroup1;
	FName SelectedGroup2;
};
