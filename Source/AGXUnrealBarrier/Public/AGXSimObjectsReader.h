// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Containers/UnrealString.h"

struct FSimulationObjectCollection;


namespace FAGXSimObjectsReader
{
	AGXUNREALBARRIER_API bool ReadAGXArchive(
		const FString& Filename, FSimulationObjectCollection& OutSimObjects);

	AGXUNREALBARRIER_API bool ReadUrdf(
		const FString& UrdfFilePath, const FString& UrdfPackagePath,
		FSimulationObjectCollection& OutSimObjects);
};
