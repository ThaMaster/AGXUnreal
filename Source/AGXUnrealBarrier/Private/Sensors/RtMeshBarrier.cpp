// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtMeshBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

FRtMeshBarrier::FRtMeshBarrier()
	: NativeRef {new FRtMeshRef}
{
}

FRtMeshBarrier::FRtMeshBarrier(std::unique_ptr<FRtMeshRef> Native)
	: NativeRef(std::move(Native))
{
}

FRtMeshBarrier::FRtMeshBarrier(FRtMeshBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtMeshRef);
}

FRtMeshBarrier::~FRtMeshBarrier()
{
}

bool FRtMeshBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FRtMeshBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	check(!HasNative());
	const agx::Vec3Vector VerticesAGX = ConvertVertices(Vertices);
	const agx::UInt32Vector IndicesAGX = ConvertIndices(Indices);
	NativeRef->Native = agxSensor::RtMesh::create(VerticesAGX, IndicesAGX);
}

FRtMeshRef* FRtMeshBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtMeshRef* FRtMeshBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtMeshBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
