
// AGXUnrealBarrier includes.
#include "TerrainBarrier.h"
#include "HeightFieldShapeBarrier.h"
#include "AGXRefs.h"
#include "TypeConversions.h"
#include "Shapes/ShapeBarrierImpl.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/HeightField.h>
#include "EndAGXIncludes.h"

#define FORCE_INITIALIZE_OPEN_VDB 0
#if FORCE_INITIALIZE_OPEN_VDB
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
			dlopen("/media/s2000/agx/master/unreal_compatible/lib/libvdbgrid.so.2.28.0.0", RTLD_NOW | RTLD_GLOBAL);
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
	// std::uniue_ptr NativeRef's destructor must be able to see the definition,
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
	agxCollide::HeightField* HeightFieldAGX = SourceHeightField.GetNativeShape<agxCollide::HeightField>();
	NativeRef->Native = agxTerrain::Terrain::createFromHeightField(HeightFieldAGX, MaximumDepth);
	UE_LOG(LogTemp, Log, TEXT("Native terrain allocated."));
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
	NativeRef->Native = nullptr;
}
