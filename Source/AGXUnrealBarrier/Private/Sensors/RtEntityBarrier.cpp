#include "Sensors/RtEntityBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/RtMeshBarrier.h"
#include "Sensors/SensorRef.h"


FRtEntityBarrier::FRtEntityBarrier()
	: NativeRef {new FRtEntityRef}
{
}

FRtEntityBarrier::FRtEntityBarrier(std::unique_ptr<FRtEntityRef> Native)
	: NativeRef(std::move(Native))
{
}

FRtEntityBarrier::FRtEntityBarrier(FRtEntityBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtEntityRef);
}

FRtEntityBarrier::~FRtEntityBarrier()
{
	ReleaseNative();
}

bool FRtEntityBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FRtEntityBarrier::AllocateNative(FRtMeshBarrier& Mesh)
{
	check(!HasNative());
	check(Mesh.HasNative());

	NativeRef->Native = agxSensor::RtEntity::create(Mesh.GetNative()->Native.get());
}

FRtEntityRef* FRtEntityBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtEntityRef* FRtEntityBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtEntityBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}
