// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtMeshEntityBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/RtMeshBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

FRtMeshEntityBarrier::FRtMeshEntityBarrier()
	: NativeRef {new FRtMeshEntity}
{
}

FRtMeshEntityBarrier::FRtMeshEntityBarrier(std::unique_ptr<FRtMeshEntity> Native)
	: NativeRef(std::move(Native))
{
}

FRtMeshEntityBarrier::FRtMeshEntityBarrier(FRtMeshEntityBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtMeshEntity);
}

FRtMeshEntityBarrier::~FRtMeshEntityBarrier()
{
}

bool FRtMeshEntityBarrier::HasNative() const
{
	return NativeRef->Native.instanceHandle != nullptr;
}

void FRtMeshEntityBarrier::AllocateNative(FRtMeshBarrier& Mesh)
{
	check(!HasNative());
	check(Mesh.HasNative());

	NativeRef->Native = agxSensor::RtMeshEntity::create(Mesh.GetNative()->Native, {}); // TODO: EntityID!
}

FRtMeshEntity* FRtMeshEntityBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtMeshEntity* FRtMeshEntityBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtMeshEntityBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	NativeRef->Native.setTransform(
		ConvertMatrix(Transform.GetLocation(), Transform.GetRotation()),
		Convert(Transform.GetScale3D()));
}
