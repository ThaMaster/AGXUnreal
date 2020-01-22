#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"

#include "AGX_CollisionGroupPair.h"

#include "AGX_CollisionGroupManager.generated.h"

/**
 * Defines for which collision groups collision should be disabled.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API AAGX_CollisionGroupManager : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "AGX Collision Group Manager")
	TArray<FAGX_CollisionGroupPair> DisabledCollisionGroups;

	void DisableSelectedCollisionGroupPairs();

	void ReenableSelectedCollisionGroupPairs();

	void UpdateAvailableCollisionGroups();

	TArray<FName> GetAvailableCollisionGroups() const { return AvailableCollisionGroups; }

	FName& GetSelectedGroup1() { return SelectedGroup1; }

	FName& GetSelectedGroup2() { return SelectedGroup2; }

	static AAGX_CollisionGroupManager* GetFrom(UWorld* World);

protected:
	virtual void BeginPlay() override;

private:
	void AddCollisionGroupPairsToSimulation();

	TArray<FName> GetAllAvailableCollisionGroupsFromWorld();

	bool CollisionGroupPairDisabled(
		FName CollisionGroup1, FName CollisionGroup2, int& OutIndex);

	void RemoveDeprecatedCollisionGroups();

private:
	TArray<FName> AvailableCollisionGroups;
	FName SelectedGroup1;
	FName SelectedGroup2;
};
