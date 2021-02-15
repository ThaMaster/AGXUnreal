#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"

// AGX Dynamics for Unreal includes.
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"

#define LOCTEXT_NAMESPACE "AAGX_CollisionGroupDisablerActor"

AAGX_CollisionGroupDisablerActor::AAGX_CollisionGroupDisablerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	CollisionGroupDisablerComponent = CreateDefaultSubobject<UAGX_CollisionGroupDisablerComponent>(
		TEXT("AGX_CollisionGroupDisabler"));
}

#undef LOCTEXT_NAMESPACE
