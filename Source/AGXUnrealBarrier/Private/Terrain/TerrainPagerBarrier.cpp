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

	bool DoesExistModifiedHeights(const TerrainPager::TileAttachmentPtrVector& ActiveTiles)
	{
		for (TerrainPager::TileAttachments* Tile : ActiveTiles)
		{
			if (Tile == nullptr || Tile->m_terrainTile == nullptr)
				continue;

			// getModifiedVertices simply returns a reference to a member of AGX Terrain, i.e. is
			// fast.
			if (Tile->m_terrainTile->getModifiedVertices().size() > 0)
				return true;
		}

		return false;
	}
}

bool FTerrainPagerBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FTerrainPagerBarrier::AllocateNative(
	FTerrainHeightFetcherBase* HeightFetcher, FTerrainBarrier& TerrainBarrier,
	int32 TileSideVertices, int32 TileOverlapVerties, double ElementSize, double MaxDepth)
{
	check(TerrainBarrier.HasNative());

	if (HeightFetcher == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("TerrainPager got nullptr HeightFetcher when allocating native. AGX Dynamics will "
				 "not be able to fetch heights from the Landscape."));
	}

	// Create a TerrainDataSource and assign the HeightFetcher to it. This HeightFetcher is owned by
	// the UAGX_Terrain and is a way for us to call UAGX_Terrain::FetchHeights from the Barrier
	// module.
	DataSourceRef = std::make_unique<FTerrainDataSourceRef>();
	DataSourceRef->Native = new FTerrainDataSource();
	DataSourceRef->Native->SetTerrainHeightFetcher(HeightFetcher);

	// Use the same position/rotation as the given Terrain.
	const agx::Vec3 Position = ConvertDisplacement(TerrainBarrier.GetPosition());
	const agx::Quat Rotation = Convert(TerrainBarrier.GetRotation());
	const agx::Real ElementSizeAGX = ConvertDistanceToAGX(ElementSize);
	const agx::Real MaxDepthAGX = ConvertDistanceToAGX(MaxDepth);

	NativeRef->Native = new agxTerrain::TerrainPager(
		TileSideVertices, TileOverlapVerties, ElementSizeAGX, MaxDepthAGX, Position, Rotation,
		TerrainBarrier.GetNative()->Native);

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

TArray<std::tuple<int32, int32>> FTerrainPagerBarrier::GetModifiedHeights(
	TArray<float>& OutHeights, int32 BoundVertsX, int32 BoundVertsY) const
{
	using namespace TerrainPagerBarrier_helpers;
	check(HasNative());

	TArray<std::tuple<int32, int32>> ModifiedVertices;
	const TerrainPager::TileAttachmentPtrVector ActiveTiles =
		NativeRef->Native->getActiveTileAttachments();

	if (!DoesExistModifiedHeights(ActiveTiles))
		return ModifiedVertices;

	const int32 BoundsCornerToCenterOffsX = BoundVertsX / 2;
	const int32 BoundsCornerToCenterOffsY = BoundVertsY / 2;

	const agxTerrain::TileSpecification& TileSpec = NativeRef->Native->getTileSpecification();
	const int32 NumVertsPerTile = static_cast<int32>(TileSpec.getTileResolution());
	const int32 TileOverlap = static_cast<int32>(TileSpec.getTileMarginSize());

	for (TerrainPager::TileAttachments* Tile : ActiveTiles)
	{
		if (Tile == nullptr || Tile->m_terrainTile == nullptr)
			continue;

		agxTerrain::TileId Id =
			TileSpec.convertWorldCoordinateToTileId(Tile->m_terrainTile->getPosition());

		const int32 CenterToTileOffsetX = Id.x() * ((NumVertsPerTile - 1) - TileOverlap);
		const int32 CenterToTileOffsetY =
			-Id.y() * ((NumVertsPerTile - 1) - TileOverlap); // Flip y axis.

		const auto& ModifiedVerticesAGX = Tile->m_terrainTile->getModifiedVertices();
		for (const auto& Index2d : ModifiedVerticesAGX)
		{
			const int32 X = BoundsCornerToCenterOffsX + CenterToTileOffsetX + Index2d.x();
			const int32 Y = BoundsCornerToCenterOffsY + CenterToTileOffsetY - Index2d.y();

			ModifiedVertices.Add(std::tuple(X, Y));
			OutHeights[X + Y * BoundVertsX] = ConvertDistanceToUnreal<float>(
				Tile->m_terrainTile->getHeightField()->getHeight(Index2d.x(), Index2d.y()));
		}
	}

	return ModifiedVertices;
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
