#include "CollisionGroups/AGX_DisabledCollisionGroupsComponent.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "CollisionGroups/AGX_CollisionGroupManager.h"

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
		if (DisabledCollisions == nullptr)
		{
			/// \todo This fails and prints an error message while in the Blueprint editor. We would
			/// like to detect that case and bail early to prevent the error message.
			/// How can we detect that SpawnActor will fail?
			DisabledCollisions = World->SpawnActor<AAGX_CollisionGroupManager>();
		}
		if (DisabledCollisions == nullptr)
		{
			// This is currently printed when in the Blueprint editor. We want to detect that and
			// bail before we get this far.
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot import disabled collision group pairs because there is no "
					 "CollisionGroupManager in the level."));
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
