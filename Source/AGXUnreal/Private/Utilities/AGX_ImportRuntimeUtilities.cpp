// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportRuntimeUtilities.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"

void FAGX_ImportRuntimeUtilities::WriteSessionGuid(
	UActorComponent& Component, const FGuid& SessionGuid)
{
	Component.ComponentTags.Empty();
	Component.ComponentTags.Add(*SessionGuid.ToString());
}
