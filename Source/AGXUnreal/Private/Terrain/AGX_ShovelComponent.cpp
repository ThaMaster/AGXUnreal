// Copyright 2023, Algoryx Simulation AB.


#include "Terrain/AGX_ShovelComponent.h"


// Sets default values for this component's properties
UAGX_ShovelComponent::UAGX_ShovelComponent()
{
	// Keep ticking off until we have a reason to turn it on.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAGX_ShovelComponent::BeginPlay()
{
	Super::BeginPlay();

	/// @todo Call createNative.
}
