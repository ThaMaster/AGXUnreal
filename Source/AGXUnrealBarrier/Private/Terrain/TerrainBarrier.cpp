#include "Terrain/TerrainBarrier.h"

// AGXUnrealBarrier includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "TypeConversions.h"
#include "Shapes/ShapeBarrierImpl.h"
#include "Terrain/ShovelBarrier.h"
#include "Materials/TerrainMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/HeightField.h>
#include <agx/Physics/GranularBodySystem.h>
#include "EndAGXIncludes.h"

/// \todo This block of code is for linker debugging only. Should be removed
/// before the first public release. There is a related piece of code in
/// AGXUnrealLibary.Build.cs, where AGXUnrealLibrary is linked against vdbgrid
/// and openvdb.
#if defined(__linux__)
#define FORCE_INITIALIZE_OPEN_VDB 1
#else
#define FORCE_INITIALIZE_OPEN_VDB 0
#endif
#if FORCE_INITIALIZE_OPEN_VDB
#define FORCE_DLOPEN_VDB_GRID 0
#define FORCE_DLOPEN_OPEN_VDB 0
#include <iostream>
#include <agx/PushDisableWarnings.h>
#include <openvdb/openvdb.h>
namespace
{
	struct FForceInitializeOpenVDB
	{
		void DoForceInitializeOpenVDB()
		{
			std::cout << "Using std::cout" << std::endl;
#if FORCE_DLOPEN_OPEN_VDB
			dlopen(
				"/media/s2000/agx/master/unreal_dependencies/lib/libvdbgrid.so.2.28.0.0",
				RTLD_NOW | RTLD_GLOBAL);
#endif
#if FORCE_DLOPEN_VDB_GRID
			dlopen(
				"/media/s2000/agx/master/unreal_dependencies/dependencies/"
				"agxTerrain_dependencies_20191204_ubuntu_18.04_64_unreal/lib/libopenvdb.so",
				RTLD_NOW | RTLD_GLOBAL);
#endif
			openvdb::initialize();
		}
	} ForceInitializeOpenVDB;
}
#include <agx/PopDisableWarnings.h>
#endif

FTerrainBarrier::FTerrainBarrier()
	: NativeRef {new FTerrainRef}
{
}

FTerrainBarrier::FTerrainBarrier(std::unique_ptr<FTerrainRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FTerrainBarrier::FTerrainBarrier(FTerrainBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTerrainBarrier::~FTerrainBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainRef.
}

bool FTerrainBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FTerrainBarrier::AllocateNative(FHeightFieldShapeBarrier& SourceHeightField)
{
#if FORCE_INITIALIZE_OPEN_VDB
	ForceInitializeOpenVDB.DoForceInitializeOpenVDB();
#endif

	check(!HasNative());
	agx::Real MaximumDepth {10.0};
	agxCollide::HeightField* HeightFieldAGX =
		SourceHeightField.GetNativeShape<agxCollide::HeightField>();
	NativeRef->Native = agxTerrain::Terrain::createFromHeightField(HeightFieldAGX, MaximumDepth);
	UE_LOG(LogAGX, Log, TEXT("Native terrain allocated."));
}

FTerrainRef* FTerrainBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainRef* FTerrainBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTerrainBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

bool FTerrainBarrier::AddShovel(FShovelBarrier& Shovel)
{
	check(HasNative());
	check(Shovel.HasNative());
	return NativeRef->Native->add(Shovel.GetNative()->Native);
}

void FTerrainBarrier::SetShapeMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->Native->setMaterial(Material.GetNative()->Native);
}

void FTerrainBarrier::SetTerrainMaterial(const FTerrainMaterialBarrier& TerrainMaterial)
{
	check(HasNative());
	check(TerrainMaterial.HasNative());
	NativeRef->Native->setTerrainMaterial(TerrainMaterial.GetNative()->Native);
}

int32 FTerrainBarrier::GetGridSizeX() const
{
	check(HasNative());
	size_t GridSize = NativeRef->Native->getResolutionX();
	check(GridSize < static_cast<size_t>(std::numeric_limits<int32>::max()));
	return static_cast<int32>(GridSize);
}

int32 FTerrainBarrier::GetGridSizeY() const
{
	check(HasNative());
	size_t GridSize = NativeRef->Native->getResolutionY();
	check(GridSize < static_cast<size_t>(std::numeric_limits<int32>::max()));
	return static_cast<int32>(GridSize);
}

TArray<float> FTerrainBarrier::GetHeights() const
{
	check(HasNative());
	const agxCollide::HeightField* HeightField = NativeRef->Native->getHeightField();
	const size_t SizeXAGX = HeightField->getResolutionX();
	const size_t SizeYAGX = HeightField->getResolutionY();

	TArray<float> Heights;
	if (SizeXAGX == 0 || SizeYAGX == 0)
	{
		/// \todo Unclear if this really should be a warning or not. When would
		/// zero-sized terrains be used?
		UE_LOG(LogAGX, Warning, TEXT("Cannot get heights from terrain with zero size."));
		return Heights;
	}
	if (SizeXAGX * SizeYAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot get heights, terrain has too many vertices."));
		return Heights;
	}

	int32 SizeX = static_cast<int32>(SizeXAGX);
	int32 SizeY = static_cast<int32>(SizeYAGX);

	Heights.Reserve(SizeX * SizeY);

	// AGX Dynamics and Unreal have different coordinate systems, so we must
	// flip the Y axis for the vertex locations.
	//
	// AGX Dynamics has [0,0] in the bottom left:
	//
	// [0,n-1] [1,n-1] ...      This is
	// ...                      what we
	// [0,2] [1,2] [3,2] ...    read from.
	// [0,1] [1,1] [2,1] ...
	// [0,0] [1,0] [2,0] ...
	//
	//
	// Unreal has [0,0] in the top left:
	//
	// [0,0] [1,0] [2,0] ...    This is
	// [0,1] [1,1] [2,1] ...    what we
	// [0,2] [1,2] [2,2] ...    write to.
	// ...
	// [0,n-1] [1,n-1] ...
	//
	// When looking at the Landscape in Unreal Editor, the user sees the axis
	// widget aligned with the Unreal section above, with the red/green/blue
	// lines at the [0,0] vertex.
	//
	// We want the ordering of the returned array to be row major according to
	// the Unreal figure above. Thus, we start by reading the last/top row of
	// increasing X coordinates, i.e., the row with Y=n-1, from AGX Dynamics'
	// point of view.
	for (int32 Y = SizeY - 1; Y >= 0; Y--)
	{
		for (int32 X = 0; X < SizeX; ++X)
		{
			Heights.Add(ConvertDistance(HeightField->getHeight(X, Y)));
		}
	}
	return Heights;
}

namespace
{
	const agx::Physics::GranularBodyPtrArray GetParticles(const agxTerrain::Terrain& Terrain)
	{
		return Terrain.getSoilSimulationInterface()->getGranularBodySystem()->getParticles();
	}

	template <typename T>
	TArray<T> CreateAndUninitializeParticleArray(
		const agx::Physics::GranularBodyPtrArray& GranularParticles)
	{
		TArray<T> Array;
		const size_t NumParticlesAGX = FMath::Clamp(
			GranularParticles.size(), size_t {0},
			static_cast<size_t>(std::numeric_limits<int32>::max()));

		if (NumParticlesAGX == 0)
		{
			return Array;
		}

		const int32 NumParticles = static_cast<int32>(NumParticlesAGX);
		Array.AddUninitialized(NumParticles);
		return Array;
	}
}

TArray<FVector> FTerrainBarrier::GetParticlePositions() const
{
	/// \todo Compare performance of this implementation with loop over
	/// granularBodySystem->getParticleStorage()->getBuffer("position")->getArray<Vec3>()
	check(HasNative());
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetParticles(*NativeRef->Native);
	TArray<FVector> Positions = CreateAndUninitializeParticleArray<FVector>(GranularParticles);
	for (int32 i = 0; i < Positions.Num(); ++i)
	{
		const agx::Vec3 PositionAGX = GranularParticles[i].position();
		const FVector Position = ConvertVector(PositionAGX);
		Positions[i] = Position;
	}

	return Positions;
}

TArray<float> FTerrainBarrier::GetParticleRadii() const
{
	check(HasNative());
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetParticles(*NativeRef->Native);
	TArray<float> Radii = CreateAndUninitializeParticleArray<float>(GranularParticles);
	for (int32 i = 0; i < Radii.Num(); ++i)
	{
		const agx::Real RadiusAGX = GranularParticles[i].radius();
		const float Radius = ConvertDistance(RadiusAGX);
		Radii[i] = Radius;
	}
	return Radii;
}

TArray<FQuat> FTerrainBarrier::GetParticleRotations() const
{
	check(HasNative());
	const agx::Physics::GranularBodyPtrArray GranularParticles = GetParticles(*NativeRef->Native);
	TArray<FQuat> Rotations = CreateAndUninitializeParticleArray<FQuat>(GranularParticles);
	for (int32 i = 0; i < Rotations.Num(); ++i)
	{
		const agx::Quat RotationAGX = GranularParticles[i].rotation();
		const FQuat Rotation = Convert(RotationAGX);
		Rotations[i] = Rotation;
	}
	return Rotations;
}
