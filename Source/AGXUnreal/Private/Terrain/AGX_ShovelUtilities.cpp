// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelUtilities.h"

// Unreal Engine includes.
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
