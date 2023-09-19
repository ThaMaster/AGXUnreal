// Copyright 2023, Algoryx Simulation AB.

#include "Materials/MaterialLibraryBarrier.h"

// AGX Dynamics for Unreal includes.
#include <TypeConversions.h>
#include <AGXBarrierFactories.h>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxTerrain/TerrainMaterialLibrary.h>
#include <agxUtil/agxUtil.h>
#include <EndAGXIncludes.h>

TArray<FString> AGX_MaterialLibraryBarrier::GetAvailableLibraryMaterials()
{
	agx::StringVector NamesAGX =
		agxTerrain::TerrainMaterialLibrary::getAvailableLibraryMaterials();
	TArray<FString> NamesUnreal;
	NamesUnreal.Reserve(NamesAGX.size());
	for (const agx::String& NameAGX : NamesAGX)
	{
		NamesUnreal.Add(Convert(NameAGX));
	}

	// Must be called to avoid crash due to different allocators used by AGX Dynamics and
	// Unreal Engine.
	agxUtil::freeContainerMemory(NamesAGX);

	return NamesUnreal;
}

FTerrainMaterialBarrier AGX_MaterialLibraryBarrier::LoadMaterialProfile(
	const FString& MaterialName)
{
	const agx::String MaterialNameAGX = Convert(MaterialName);
	agxTerrain::TerrainMaterialRef Material = new agxTerrain::TerrainMaterial(MaterialNameAGX);
	agxTerrain::TerrainMaterialLibrary::loadMaterialProfile(MaterialNameAGX, Material);
	return AGXBarrierFactories::CreateTerrainMaterialBarrier(Material);
}
