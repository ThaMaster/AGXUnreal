// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtShapeInstanceBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/RtShapeBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/EnvironmentInternalData.h>
#include <agxSensor/RaytraceInstanceData.h>
#include <agxSensor/RaytraceSurfaceMaterial.h>
#include "EndAGXIncludes.h"

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier()
	: NativeRef {new FRtShapeInstance}
{
}

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier(std::unique_ptr<FRtShapeInstance> Native)
	: NativeRef(std::move(Native))
{
}

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier(FRtShapeInstanceBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtShapeInstance);
}

FRtShapeInstanceBarrier::~FRtShapeInstanceBarrier()
{
}

bool FRtShapeInstanceBarrier::HasNative() const
{
	return NativeRef->Native.handle != nullptr;
}

void FRtShapeInstanceBarrier::AllocateNative(
	FRtShapeBarrier& Shape, FSensorEnvironmentBarrier& Environment, float Reflectivity)
{
	check(!HasNative());
	check(Shape.HasNative());
	check(Environment.HasNative());

	NativeRef->Native = agxSensor::RtShapeInstance::create(
		Environment.GetNative()->Native->getScene(),
		Shape.GetNative()->Native, nullptr);
}

FRtShapeInstance* FRtShapeInstanceBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtShapeInstance* FRtShapeInstanceBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtShapeInstanceBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	NativeRef->Native.setTransform(
		ConvertMatrix(Transform.GetLocation(), Transform.GetRotation()),
		Convert(Transform.GetScale3D()));
}
