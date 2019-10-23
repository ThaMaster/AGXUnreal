#include "TrimeshShapeBarrier.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"

#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"


namespace
{
	agxCollide::Trimesh* NativeTrimesh(FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}

	const agxCollide::Trimesh* NativeTrimesh(const FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier()
	: FShapeBarrier()
{
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Trimesh>());
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}


FTrimeshShapeBarrier::~FTrimeshShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrimeshShapeRef.
}


void FTrimeshShapeBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise, UWorld* World)
{
	{
		// Create temporary allocation parameters structure for AllocateNativeShape() to use.

		std::shared_ptr<AllocationParameters> Params = std::make_shared<AllocationParameters>();
		Params->Vertices = &Vertices;
		Params->TriIndices = &TriIndices;
		Params->bClockwise = bClockwise;
		Params->World = World;

		TemporaryAllocationParameters = Params;

		FShapeBarrier::AllocateNative(); // Will implicitly invoke AllocateNativeShape(). See below.

	}
	// Temporary allocation parameters structure destroyed by smart pointer.
}


void FTrimeshShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());

	// Retrieve the temporary allocation parameters.

	std::shared_ptr<AllocationParameters> Params = TemporaryAllocationParameters.lock();
	check(Params != nullptr);
	
	// Transfer to native buffers.

	agx::Vec3Vector NativeVertices;
	NativeVertices.reserve(Params->Vertices->Num());

	for (const FVector& Vertex : *Params->Vertices)
	{
		NativeVertices.push_back(ConvertDistance(Vertex, Params->World));
	}

	agx::UInt32Vector NativeIndices;
	NativeIndices.reserve(Params->TriIndices->Num() * 3);

	for (const FTriIndices& Index : *Params->TriIndices)
	{
		check(Index.v0 >= 0);
		check(Index.v1 >= 0);
		check(Index.v2 >= 0);

		NativeIndices.push_back(static_cast<uint32>(Index.v0));
		NativeIndices.push_back(static_cast<uint32>(Index.v1));
		NativeIndices.push_back(static_cast<uint32>(Index.v2));
	}

	agxCollide::Trimesh::TrimeshOptionsFlags OptionsMask = Params->bClockwise ?
		agxCollide::Trimesh::TrimeshOptionsFlags::CLOCKWISE_ORIENTATION :
		static_cast<agxCollide::Trimesh::TrimeshOptionsFlags>(0);

	// Create the native object.

	NativeRef->NativeShape = new agxCollide::Trimesh(
		&NativeVertices, &NativeIndices, "FTrimeshShapeBarrier", OptionsMask);
}

void FTrimeshShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
