#include "Terrain/TerrainPagerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainBarrier.h"
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

void FTerrainPagerBarrier::AllocateNative(
	FTerrainHeightFetcherBase* HeightFetcher, FTerrainBarrier& TerrainBarrier)
{
	check(TerrainBarrier.HasNative());

	// Create a TerrainDataSource and assign the HeightFetcher to it. This HeightFetcher is owned by
	// the UAGX_Terrain and is a way for us to call UAGX_Terrain::FetchHeights from the Barrier
	// module.
	DataSourceRef = std::make_unique<FTerrainDataSourceRef>();
	DataSourceRef->Native = new FTerrainDataSource();
	DataSourceRef->Native->SetTerrainHeightFetcher(HeightFetcher);

	NativeRef->Native = new agxTerrain::TerrainPager(
		101, 10, 1.0, 3.0, agx::Vec3(), agx::Quat(), TerrainBarrier.GetNative()->Native);

	NativeRef->Native->setTerrainDataSource(DataSourceRef->Native);
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
