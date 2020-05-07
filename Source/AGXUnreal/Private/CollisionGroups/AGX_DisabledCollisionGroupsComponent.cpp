#include "CollisionGroups/AGX_DisabledCollisionGroupsComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "CollisionGroups/AGX_CollisionGroupManager.h"
#include "Engine/World.h"

namespace
{
	AAGX_CollisionGroupManager* GetCollisionGroupManager(AActor* Owner)
	{
		if (Owner == nullptr)
		{
			return nullptr;
		}
		UWorld* World = Owner->GetWorld();
		if (World == nullptr)
		{
			return nullptr;
		}
		AAGX_CollisionGroupManager* DisabledCollisions = AAGX_CollisionGroupManager::GetFrom(World);
		if (DisabledCollisions == nullptr)
		{
			return nullptr;
		}
		return DisabledCollisions;
	}

	bool AddDisabledPairs(AActor* Owner, TArray<FAGX_CollisionGroupPair>& PairsToDisable)
	{
		AAGX_CollisionGroupManager* DisabledCollisions = GetCollisionGroupManager(Owner);
		if (DisabledCollisions == nullptr)
		{
			return false;
		}

		for (const FAGX_CollisionGroupPair& Pair : PairsToDisable)
		{
			DisabledCollisions->DisableCollisionGroupPair(Pair);
		}
		return true;
	}
}

UAGX_DisabledCollisionGroupsComponent::UAGX_DisabledCollisionGroupsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
};

void UAGX_DisabledCollisionGroupsComponent::PostLoad()
{
	Super::PostLoad();
	if (bPairsDisabled)
	{
		return;
	}
	if (AddDisabledPairs(GetOwner(), DisabledCollisionGroupPairs))
	{
		bPairsDisabled = true;
		SetComponentTickEnabled(false);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_DisableCollisionGroupsComponent in '%s' could not register disabled "
				 "collision groups in PostLoad because the current level doesn't have an "
				 "AGX_CollisionGroupManager. Will try again later."),
			*GetOwner()->GetName());
	}
}

void UAGX_DisabledCollisionGroupsComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bPairsDisabled)
	{
		return;
	}
	if (AddDisabledPairs(GetOwner(), DisabledCollisionGroupPairs))
	{
		bPairsDisabled = true;
		SetComponentTickEnabled(false);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"AGX_DisableCollisionGroupsComponent in '%s' could not register disabled collision "
				"groups in BeginPlay because the current level doesn't have an "
				"AGX_CollisionGroupManager. Will try again later."),
			*GetOwner()->GetName());
	}
}

void UAGX_DisabledCollisionGroupsComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only making one attempt from the tick callback. If we don't have a collision group
	// manager by now then we probably never will.
	SetComponentTickEnabled(false);

	if (bPairsDisabled)
	{
		return;
	}

	if (AddDisabledPairs(GetOwner(), DisabledCollisionGroupPairs))
	{
		bPairsDisabled = true;
	}
	else
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_DisableCollisionGroupsComponent in '%s' could not register disabled "
				 "collision groups in TickComponent because there is no CollisionGroupManager "
				 "in the level. No further attempts will be made."),
			*GetOwner()->GetName());
	}
}
