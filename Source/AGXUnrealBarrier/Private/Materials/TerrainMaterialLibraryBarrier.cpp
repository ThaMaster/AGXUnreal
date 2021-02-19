#include "Materials/TerrainMaterialLibraryBarrier.h"

// AGX Dynamics for Unreal includes.
#include <TypeConversions.h>
#include <AGXBarrierFactories.h>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxTerrain/TerrainMaterialLibrary.h>
#include <EndAGXIncludes.h>

TArray<FString> AGX_TerrainMaterialLibraryBarrier::GetAvailableLibraryMaterials()
{
	const agx::StringVector NamesAGX = agxTerrain::TerrainMaterialLibrary::getAvailableLibraryMaterials();
	TArray<FString> NamesUnreal;
	NamesUnreal.Reserve(NamesAGX.size());
	for (const agx::String& NameAGX : NamesAGX)
	{
		NamesUnreal.Add(Convert(NameAGX));
	}
	return NamesUnreal;
}

FTerrainMaterialBarrier AGX_TerrainMaterialLibraryBarrier::LoadMaterialProfile(
	const FString& MaterialName)
{
	const agx::String MaterialNameAGX = Convert(MaterialName);
	agxTerrain::TerrainMaterialRef Material = new agxTerrain::TerrainMaterial(MaterialNameAGX);
	agxTerrain::TerrainMaterialLibrary::loadMaterialProfile(MaterialNameAGX, Material);
	return AGXBarrierFactories::CreateTerrainMaterialBarrier(Material);
}
