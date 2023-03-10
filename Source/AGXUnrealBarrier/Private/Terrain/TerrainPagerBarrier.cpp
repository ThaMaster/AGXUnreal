#include "Terrain/TerrainPagerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainDataSource.h"
#include "Terrain/TerrainHeightFetcherBase.h"
#include "TypeConversions.h"
#include "Utilities/TerrainUtilities.h"

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

namespace TerrainPagerBarrier_helpers
{
	size_t GetNumParticles(const agxTerrain::Terrain* Terrain)
	{
		if (Terrain == nullptr)
			return 0;

		return Terrain->getSoilSimulationInterface()->getGranularBodySystem()->getNumParticles();
	}

	size_t GetNumParticles(const agxTerrain::TerrainPager::TileAttachmentPtrVector& ActiveTiles)
	{
		size_t NumParticles = 0;
		for (TerrainPager::TileAttachments* Tile : ActiveTiles)
		{
			if (Tile == nullptr || Tile->m_terrainTile == nullptr)
				continue;

			NumParticles += GetNumParticles(Tile->m_terrainTile.get());
		}

		return NumParticles;
	}
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

	// Use the same position/rotation as the given Terrain.
	const agx::Vec3 Position = ConvertDisplacement(TerrainBarrier.GetPosition());
	const agx::Quat Rotation = Convert(TerrainBarrier.GetRotation());

	// @todo: pass proper values.
	NativeRef->Native = new agxTerrain::TerrainPager(
		51, 5, 0.2, 3.0, Position, Rotation, TerrainBarrier.GetNative()->Native);

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

bool FTerrainPagerBarrier::AddShovel(FShovelBarrier& Shovel)
{
	check(HasNative());
	check(Shovel.HasNative());

	// @todo: make radiuses an input param.
	return NativeRef->Native->add(Shovel.GetNative()->Native, 0.5, 1.0);
}

FParticleData FTerrainPagerBarrier::GetParticleData() const
{
	using namespace agxTerrain;
	check(HasNative());

	FParticleData ParticleData;
	const TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();

	const size_t NumParticles = TerrainPagerBarrier_helpers::GetNumParticles(ActiveTiles);
	ParticleData.Positions.Reserve(NumParticles);
	ParticleData.Radii.Reserve(NumParticles);
	ParticleData.Rotations.Reserve(NumParticles);

	for (TerrainPager::TileAttachments* Tile : ActiveTiles)
	{
		if (Tile == nullptr || Tile->m_terrainTile == nullptr)
			continue;

		const FTerrainBarrier TerrainBarrier =
			AGXBarrierFactories::CreateTerrainBarrier(Tile->m_terrainTile.get());

		FTerrainUtilities::AppendParticleData(TerrainBarrier, ParticleData);
	}

	return ParticleData;
}

TArray<std::tuple<int32, int32>> FTerrainPagerBarrier::GetModifiedVertices() const
{
	// @todo: implement.
	return TArray<std::tuple<int32, int32>>();
}

void FTerrainPagerBarrier::GetModifiedHeights(TArray<float>& OutHeights) const
{
	// @todo: implement.
}

FVector FTerrainPagerBarrier::GetReferencePoint() const
{
	check(HasNative());
	return ConvertDisplacement(NativeRef->Native->getTileSpecification().getReferencePoint());
}

FQuat FTerrainPagerBarrier::GetReferenceRotation() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getTileSpecification().getReferenceRotation());
}
