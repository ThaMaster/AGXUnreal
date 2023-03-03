#include "Terrain/TerrainPagerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainDataSource.h"
#include "Terrain/TerrainHeightFetcherBase.h"



FTerrainPagerBarrier::FTerrainPagerBarrier()
	: NativeRef {new FTerrainPagerRef}
{
}

FTerrainPagerBarrier::FTerrainPagerBarrier(std::unique_ptr<FTerrainPagerRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FTerrainPagerBarrier::FTerrainPagerBarrier(FTerrainPagerBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTerrainPagerBarrier::~FTerrainPagerBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainPagerRef.
}

bool FTerrainPagerBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FTerrainPagerBarrier::AllocateNative(FTerrainHeightFetcherBase* HeightFetcher)
{
	DataSource = std::make_unique<FTerrainDataSource>();
	DataSource->SetTerrainHeightFetcher(HeightFetcher);

	// @todo : create native and give it our DataSource.
}

FTerrainPagerRef* FTerrainPagerBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainPagerRef* FTerrainPagerBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTerrainPagerBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
