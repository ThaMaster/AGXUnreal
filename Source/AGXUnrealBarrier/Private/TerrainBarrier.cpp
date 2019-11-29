
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


#include <agx/PushDisableWarnings.h>
#include <openvdb/openvdb.h>
namespace
{
	struct FForceInitializeOpenVDB
	{
		FForceInitializeOpenVDB()
		{
			openvdb::initialize();
		}
	} ForceInitializeOpenVDB;
}
#include <agx/PopDisableWarnings.h>

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
