// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarResultPositionBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/AGX_LidarResultTypes.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceResult.h>
#include "EndAGXIncludes.h"

namespace LidarResultPositionBarrier_helpers
{
	struct LidarPositionData
	{
		float X;
		float Y;
		float Z;
	};
}

void FLidarResultPositionBarrier::AllocateNative()
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::RtResult();
	NativeRef->Native->add(agxSensor::RtResult::XYZ_VEC3_F32);
}

void FLidarResultPositionBarrier::GetResult(TArray<FAGX_LidarResultPositionData>& OutResult) const
{
	using namespace LidarResultPositionBarrier_helpers;

	check(HasNative());
	AGX_CHECK(sizeof(LidarPositionData) == GetNative()->Native->getElementSize());

	agxSensor::BinaryResultView<LidarPositionData> ViewAGX =
		GetNative()->Native->view<LidarPositionData>();

	OutResult.SetNumUninitialized(ViewAGX.size(), false);
	for (int32 I = 0; I < ViewAGX.size(); I++)
	{
		OutResult[I] = {ConvertDisplacement(ViewAGX[I].X, ViewAGX[I].Y, ViewAGX[I].Z)};
	}
}
