// Copyright 2022, Algoryx Simulation AB.


#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_CollisionGroupDisablerComponent::UAGX_CollisionGroupDisablerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
};

void UAGX_CollisionGroupDisablerComponent::UpdateAvailableCollisionGroups()
{
	// Reset selected collision groups to none. They may not be present
	// in the world anymore.
	SelectedGroup1 = FName(TEXT(""));
	SelectedGroup2 = FName(TEXT(""));

	UpdateAvailableCollisionGroupsFromWorld();
}

void UAGX_CollisionGroupDisablerComponent::DisableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool HideWarnings)
{
	if (Group1.IsNone() || Group2.IsNone())
	{
		if (!HideWarnings)
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLogInEditor(
				"A selected collision group may not be 'None'. Please select valid collision "
				"groups.",
				GetWorld());
		}
		return;
	}

	if (IsCollisionGroupPairDisabled(Group1, Group2))
	{
		if (!HideWarnings)
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLogInEditor(
				"Collision has already been disabled for the selected collision groups.",
				GetWorld());
		}
		return;
	}

	DisabledCollisionGroupPairs.Add(FAGX_CollisionGroupPair {Group1, Group2});

	// For the non-game world case, the groups are added to the simulation in BeginPlay().
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetWorld()))
	{
		Simulation->SetEnableCollisionGroupPair(Group1, Group2, false);
	}
}

void UAGX_CollisionGroupDisablerComponent::EnableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool HideWarnings)
{
	if (Group1.IsNone() || Group2.IsNone())
	{
		if (!HideWarnings)
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLogInEditor(
				"A selected collision group may not be 'None'. Please select valid collision "
				"groups.",
				GetWorld());
		}
		return;
	}

	int OutIndex;
	if (!IsCollisionGroupPairDisabled(Group1, Group2, OutIndex))
	{
		return;
	}

	DisabledCollisionGroupPairs.RemoveAt(OutIndex);

	// For the non-game world case, the groups are added to the simulation in BeginPlay().
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetWorld()))
	{
		Simulation->SetEnableCollisionGroupPair(Group1, Group2, true);
	}
}

void UAGX_CollisionGroupDisablerComponent::BeginPlay()
{
	Super::BeginPlay();
	AddCollisionGroupPairsToSimulation();
}

void UAGX_CollisionGroupDisablerComponent::AddCollisionGroupPairsToSimulation()
{
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetWorld()))
	{
		for (auto& Pair : DisabledCollisionGroupPairs)
		{
			Simulation->SetEnableCollisionGroupPair(Pair.Group1, Pair.Group2, false);
		}
	}
}

void UAGX_CollisionGroupDisablerComponent::UpdateAvailableCollisionGroupsFromWorld()
{
	AvailableCollisionGroups.Empty();

	// @todo Find a way to only get the shapes in the current UWorld.

	// Find all ShapeComponent objects and get all collision groups from those.
	for (TObjectIterator<UAGX_ShapeComponent> ObjectIt; ObjectIt; ++ObjectIt)
	{
		UAGX_ShapeComponent* Shape = *ObjectIt;

		if (IsValid(Shape))
		{
			continue;
		}

		for (const auto& CollisionGroup : Shape->CollisionGroups)
		{
			AvailableCollisionGroups.AddUnique(CollisionGroup);
		}
	}
}

void UAGX_CollisionGroupDisablerComponent::RemoveDeprecatedCollisionGroups()
{
	for (int i = 0; i < DisabledCollisionGroupPairs.Num();)
	{
		const FAGX_CollisionGroupPair& Pair = DisabledCollisionGroupPairs[i];
		if (AvailableCollisionGroups.IndexOfByKey(Pair.Group1) == INDEX_NONE ||
			AvailableCollisionGroups.IndexOfByKey(Pair.Group2) == INDEX_NONE)
		{
			DisabledCollisionGroupPairs.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}
}

bool UAGX_CollisionGroupDisablerComponent::IsCollisionGroupPairDisabled(
	const FName& CollisionGroup1, const FName& CollisionGroup2, int& OutIndex) const
{
	OutIndex = INDEX_NONE;

	// Check all disabled collision group pairs, and check all
	// permutations (i.e. here [A,B] == [B,A]).
	for (int i = 0; i < DisabledCollisionGroupPairs.Num(); i++)
	{
		if (DisabledCollisionGroupPairs[i].IsEqual(CollisionGroup1, CollisionGroup2))
		{
			OutIndex = i;
			return true;
		}
	}

	return false;
}

bool UAGX_CollisionGroupDisablerComponent::IsCollisionGroupPairDisabled(
	const FName& CollisionGroup1, const FName& CollisionGroup2) const
{
	int Unused;
	return IsCollisionGroupPairDisabled(CollisionGroup1, CollisionGroup2, Unused);
}
