// Copyright 2023, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordComponent.h"


// AGX Dynamics for Unreal includes.
#include "PlayRecord/AGX_PlayRecord.h"


UAGX_PlayRecordComponent::UAGX_PlayRecordComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
