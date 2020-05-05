#include "CollisionGroups/AGX_DisabledCollisionGroupsComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "CollisionGroups/AGX_CollisionGroupManager.h"
#include "Engine/World.h"

namespace
{
	void AddDisabledPairs(
		AActor* Owner, TArray<FAGX_CollisionGroupPair>& DisabledCollisionGroupPairs)
	{
		if (Owner == nullptr)
		{
			return;
		}

		UWorld* World = Owner->GetWorld();
		if (World == nullptr)
		{
			// Unclear if we should log here or not. Is there any case in which there is no World
			// but the user still need the disabled pairs to be registered in a
			// CollisionGroupManager?
			return;
		}

		/// \todo Consider adding a GetOrCreateFrom member function to AAGX_CollisionGroupManager.
		AAGX_CollisionGroupManager* DisabledCollisions = AAGX_CollisionGroupManager::GetFrom(World);
		if (DisabledCollisions == nullptr && World->bIsRunningConstructionScript)
		{
			// Not allowed to spawn new actors, such as the CollisionGroupManager, while a
			// construction script is running. Bail and try again later. This will happen while
			// importing an AGX Dynamics archive to a Blueprint. In that case "later" is when
			// the generated Blueprint is instantiated in a level.
			//
			// There is also FActorSpawnParameters.bAllowDuringConstructionScript that may be used.
			// Need to understand what the implications of setting that to true are.
			return;
		}
		if (DisabledCollisions == nullptr)
		{
			/// \todo This sometimes creates an extra CollisionGroupManager on Play. Happens when
			/// this component is instantiated before the real/original/user-created
			/// CollisionGroupManager.
			DisabledCollisions = World->SpawnActor<AAGX_CollisionGroupManager>();
		}
		if (DisabledCollisions == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("'%s' cannot apply disabled collision group pairs because there is no "
					 "CollisionGroupManager in the level."),
				*Owner->GetName());
			return;
		}

		for (const FAGX_CollisionGroupPair& Pair : DisabledCollisionGroupPairs)
		{
			DisabledCollisions->DisableCollisionGroupPair(Pair);
		}
	}
}

void UAGX_DisabledCollisionGroupsComponent::PostLoad()
{
	Super::PostLoad();
	AddDisabledPairs(GetOwner(), DisabledCollisionGroupPairs);
}

void UAGX_DisabledCollisionGroupsComponent::BeginPlay()
{
	Super::BeginPlay();
	AddDisabledPairs(GetOwner(), DisabledCollisionGroupPairs);
}
