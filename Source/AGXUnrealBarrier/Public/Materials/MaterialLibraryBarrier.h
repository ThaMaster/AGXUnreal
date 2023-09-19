// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"

namespace AGX_MaterialLibraryBarrier
{
	AGXUNREALBARRIER_API TArray<FString> GetAvailableLibraryMaterials();
	AGXUNREALBARRIER_API FTerrainMaterialBarrier LoadMaterialProfile(const FString& MaterialName);
}
