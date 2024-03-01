// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/SensorEnvironmentBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarBarrier.h"
#include "Sensors/SensorRef.h"

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier()
	: NativeRef {new FSensorEnvironmentRef}
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(std::unique_ptr<FSensorEnvironmentRef> Native)
	: NativeRef(std::move(Native))
{
}

FSensorEnvironmentBarrier::FSensorEnvironmentBarrier(FSensorEnvironmentBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FSensorEnvironmentRef);
}

FSensorEnvironmentBarrier::~FSensorEnvironmentBarrier()
{
}

bool FSensorEnvironmentBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FSensorEnvironmentBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::Environment();
}

FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSensorEnvironmentRef* FSensorEnvironmentBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FSensorEnvironmentBarrier::ReleaseNative()
{
}

bool FSensorEnvironmentBarrier::Add(FLidarBarrier& Lidar)
{
	check(HasNative());
	return NativeRef->Native->add(Lidar.GetNative()->Native);
}

void FSensorEnvironmentBarrier::Step(double DeltaTime)
{
	check(HasNative());
	NativeRef->Native->step(DeltaTime);
}
