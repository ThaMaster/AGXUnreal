#include "Materials/TerrainMaterialBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include <Misc/AssertionMacros.h>

FTerrainMaterialBarrier::FTerrainMaterialBarrier()
	: NativeRef {new FTerrainMaterialRef}
{
}

FTerrainMaterialBarrier::~FTerrainMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainMaterialRef.
}

bool FTerrainMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTerrainMaterialBarrier::AllocateNative(const FString& Name)
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainMaterial(Convert(Name));
}

void FTerrainMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainMaterialBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FTerrainMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

void FTerrainMaterialBarrier::SetDensity(double Density)
{
	check(HasNative());
	//NativeRef->Native->getBulkMaterial()->setDensity(Density);
}

double FTerrainMaterialBarrier::GetDensity() const
{
	check(HasNative());
	return 0.0;// NativeRef->Native->getBulkMaterial()->getDensity();
}
