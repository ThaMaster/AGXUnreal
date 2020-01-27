#include "CollisionGroups/AGX_CollisionGroupManager.h"

#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_ShapeComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_CollisionGroupManager"

AAGX_CollisionGroupManager::AAGX_CollisionGroupManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> ViewportIconTextureObject;
			FName Id;
			FText Name;

			FConstructorStatics()
				: ViewportIconTextureObject(TEXT("/Engine/EditorResources/S_Note"))
				, Id(TEXT("AGX_CollisionGroups"))
				, Name(LOCTEXT("ViewportIcon", "AGX Collision Groups"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (GetSpriteComponent())
		{
			GetSpriteComponent()->Sprite = ConstructorStatics.ViewportIconTextureObject.Get();
			GetSpriteComponent()->SpriteInfo.Category = ConstructorStatics.Id;
			GetSpriteComponent()->SpriteInfo.DisplayName = ConstructorStatics.Name;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void AAGX_CollisionGroupManager::DisableSelectedCollisionGroupPairs()
{
	if (SelectedGroup1.IsNone() || SelectedGroup2.IsNone())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("A selected collision group may not be 'None'. Please select valid collision "
				 "groups."));
		return;
	}

	int OutIndex;
	if (CollisionGroupPairDisabled(SelectedGroup1, SelectedGroup2, OutIndex))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Collision has already been disabled for the selected collision groups."));
		return;
	}

	DisabledCollisionGroups.Add(FAGX_CollisionGroupPair { SelectedGroup1, SelectedGroup2 });
}

void AAGX_CollisionGroupManager::ReenableSelectedCollisionGroupPairs()
{
	if (SelectedGroup1.IsNone() || SelectedGroup2.IsNone())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("A selected collision group may not be 'None'. Please select valid collision "
				 "groups."));
		return;
	}

	int OutIndex;
	if (CollisionGroupPairDisabled(SelectedGroup1, SelectedGroup2, OutIndex))
	{
		DisabledCollisionGroups.RemoveAt(OutIndex);
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Collision between the selected collision groups is not disabled."));
	}
}

void AAGX_CollisionGroupManager::UpdateAvailableCollisionGroups()
{
	// Reset selected collision groups to none. They may not be present
	// in the world anymore.
	SelectedGroup1 = FName(TEXT(""));
	SelectedGroup2 = FName(TEXT(""));

	AvailableCollisionGroups = GetAllAvailableCollisionGroupsFromWorld();

	// Remove any disabled collision group pair that contains a collision
	// group not yet present in the world.
	RemoveDeprecatedCollisionGroups();
}

void AAGX_CollisionGroupManager::BeginPlay()
{
	AddCollisionGroupPairsToSimulation();
}

void AAGX_CollisionGroupManager::AddCollisionGroupPairsToSimulation()
{
	if (DisabledCollisionGroups.Num() <= 0)
	{
		return;
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(GetWorld());

	for (auto& Pair : DisabledCollisionGroups)
	{
		Simulation->SetDisableCollisionGroupPair(
			Pair.Group1, Pair.Group2);
	}
}

TArray<FName> AAGX_CollisionGroupManager::GetAllAvailableCollisionGroupsFromWorld()
{
	TArray<FName> FoundCollisionGroups;

	// Find all ShapeComponent objects and get all collision groups
	for (TObjectIterator<UAGX_ShapeComponent> ObjectIt; ObjectIt; ++ObjectIt)
	{
		UAGX_ShapeComponent* Shape = *ObjectIt;

		if (Shape->IsPendingKill())
			continue;

		for (const auto& CollisionGroup : Shape->CollisionGroups)
		{
			FoundCollisionGroups.AddUnique(CollisionGroup);
		}
	}

	return FoundCollisionGroups;
}

bool AAGX_CollisionGroupManager::CollisionGroupPairDisabled(
	FName CollisionGroup1, FName CollisionGroup2, int& OutIndex)
{
	OutIndex = -1;

	// Check all disabled collision group pairs, and check all
	// permutations (i.e. here [A,B] == [B,A])
	for (int i = 0; i < DisabledCollisionGroups.Num(); i++)
	{
		if ((CollisionGroup1.IsEqual(DisabledCollisionGroups[i].Group1) &&
			 CollisionGroup2.IsEqual(DisabledCollisionGroups[i].Group2)) ||
			(CollisionGroup1.IsEqual(DisabledCollisionGroups[i].Group2) &&
			 CollisionGroup2.IsEqual(DisabledCollisionGroups[i].Group1)))
		{
			OutIndex = i;
			return true;
		}
	}

	return false;
}

void AAGX_CollisionGroupManager::RemoveDeprecatedCollisionGroups()
{
	for (int i = 0; i < DisabledCollisionGroups.Num();)
	{
		if (AvailableCollisionGroups.IndexOfByKey(DisabledCollisionGroups[i].Group1) ==
				INDEX_NONE ||
			AvailableCollisionGroups.IndexOfByKey(DisabledCollisionGroups[i].Group2) == INDEX_NONE)
		{
			DisabledCollisionGroups.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}
}

#undef LOCTEXT_NAMESPACE
