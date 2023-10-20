// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "Templates/UnrealTypeTraits.h"

void FAGX_ShovelUtilities::TruncateForDetailsPanel(double& Value)
{
	// See comment in header file.
	// At the time of writing the format specifier exposed for double in UnrealTypeTraits.h is %f.
	// Value = FCString::Atod(*FString::Printf(TEXT("%f"), Value));
	Value = FCString::Atod(*FString::Printf(TFormatSpecifier<double>::GetFormatSpecifier(), Value));
}

void FAGX_ShovelUtilities::TruncateForDetailsPanel(FVector& Values)
{
	TruncateForDetailsPanel(Values.X);
	TruncateForDetailsPanel(Values.Y);
	TruncateForDetailsPanel(Values.Z);
}

void FAGX_ShovelUtilities::TruncateForDetailsPanel(FRotator& Values)
{
	TruncateForDetailsPanel(Values.Pitch);
	TruncateForDetailsPanel(Values.Yaw);
	TruncateForDetailsPanel(Values.Roll);
}
