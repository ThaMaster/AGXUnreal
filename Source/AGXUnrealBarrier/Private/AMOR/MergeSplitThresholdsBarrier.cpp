// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/MergeSplitThresholdsBarrier.h"


// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "TypeConversions.h"
#include "Wire/WireBarrier.h"
#include "Wire/WireRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include "agxCollide/Geometry.h"
#include <agxSDK/MergeSplitHandler.h>
#include "EndAGXIncludes.h"


FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier()
	: NativeRef {new FMergeSplitThresholdsRef}
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native)
	: NativeRef(std::move(Native))
{
}

FMergeSplitThresholdsBarrier::FMergeSplitThresholdsBarrier(FMergeSplitThresholdsBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
}

FMergeSplitThresholdsBarrier::~FMergeSplitThresholdsBarrier()
{
}

bool FMergeSplitThresholdsBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative()
{
	return NativeRef.get();
}

const FMergeSplitThresholdsRef* FMergeSplitThresholdsBarrier::GetNative() const
{
	return NativeRef.get();
}

void FMergeSplitThresholdsBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FGuid FMergeSplitThresholdsBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

namespace MergeSplitThresholds_helpers
{
	template <typename T>
	const auto GetFrom(const T& Barrier)
	{
		AGX_CHECK(Barrier.HasNative());
		return Barrier.GetNative()->Native;
	}

	template<>
	const auto GetFrom<FShapeBarrier>(const FShapeBarrier& Shape)
	{
		AGX_CHECK(Shape.HasNativeGeometry());
		return Shape.GetNative()->NativeGeometry;
	}

	agxSDK::MergeSplitThresholds* GetFrom(
		const agxSDK::MergeSplitProperties& Msp, const FRigidBodyBarrier&)
	{
		return Msp.getContactThresholds();
	}

	agxSDK::MergeSplitThresholds* GetFrom(
		const agxSDK::MergeSplitProperties& Msp, const FShapeBarrier&)
	{
		return Msp.getContactThresholds();
	}

	agxSDK::MergeSplitThresholds* GetFrom(
		const agxSDK::MergeSplitProperties& Msp, const FConstraintBarrier&)
	{
		return Msp.getConstraintThresholds();
	}

	agxSDK::MergeSplitThresholds* GetFrom(
		const agxSDK::MergeSplitProperties& Msp, const FWireBarrier&)
	{
		return Msp.getWireThresholds();
	}
}

template <typename T>
FMergeSplitThresholdsBarrier FMergeSplitThresholdsBarrier::CreateFrom(const T& Barrier)
{
	using namespace MergeSplitThresholds_helpers;
	if (!Barrier.HasNative())
	{
		return FMergeSplitThresholdsBarrier();
	}

	const auto Msp = agxSDK::MergeSplitHandler::getProperties(GetFrom(Barrier));
	if (Msp == nullptr)
	{
		return FMergeSplitThresholdsBarrier();
	}

	auto Mst = GetFrom(*Msp, Barrier);
	if (Mst == nullptr)
	{
		return FMergeSplitThresholdsBarrier();
	}

	return FMergeSplitThresholdsBarrier(std::make_unique<FMergeSplitThresholdsRef>(Mst));
}

template AGXUNREALBARRIER_API FMergeSplitThresholdsBarrier
FMergeSplitThresholdsBarrier::CreateFrom<FRigidBodyBarrier>(const FRigidBodyBarrier&);

template AGXUNREALBARRIER_API FMergeSplitThresholdsBarrier
FMergeSplitThresholdsBarrier::CreateFrom<FShapeBarrier>(const FShapeBarrier&);

template AGXUNREALBARRIER_API FMergeSplitThresholdsBarrier
FMergeSplitThresholdsBarrier::CreateFrom<FConstraintBarrier>(const FConstraintBarrier&);

template AGXUNREALBARRIER_API FMergeSplitThresholdsBarrier
FMergeSplitThresholdsBarrier::CreateFrom<FWireBarrier>(const FWireBarrier&);
