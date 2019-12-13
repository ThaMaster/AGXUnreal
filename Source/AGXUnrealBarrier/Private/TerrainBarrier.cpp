#include "TerrainBarrier.h"

// AGXUnrealBarrier includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "HeightFieldShapeBarrier.h"
#include "TypeConversions.h"
#include "Shapes/ShapeBarrierImpl.h"
#include "Terrain/ShovelBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/HeightField.h>
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
