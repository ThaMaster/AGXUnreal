// Copyright 2023, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerSpriteComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_CollisionGroupDisablerActor"

AAGX_CollisionGroupDisablerActor::AAGX_CollisionGroupDisablerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());

	CollisionGroupDisablerComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerComponent>(
		TEXT("AGX_CollisionGroupDisabler"));
}

void AAGX_CollisionGroupDisablerActor::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);

	if (RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SetRootComponent(NewObject<UAGX_CollisionGroupDisablerSpriteComponent>(
			this, USceneComponent::GetDefaultSceneRootVariableName()));
		RootComponent->SetFlags(RF_Transactional);
		AddInstanceComponent(RootComponent);
		// We should not register the Component here because it is too early. The Component will be
		// registered automatically by this Actor.
	}
}

#undef LOCTEXT_NAMESPACE
