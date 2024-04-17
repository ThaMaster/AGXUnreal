// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/LidarResultPositionIntensityBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/AGX_LidarResultTypes.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/RaytraceResult.h>
#include "EndAGXIncludes.h"

namespace LidarResultPositionIntensityBarrier_helpers
{
	struct LidarPositionIntensityData
	{
		float X;
		float Y;
		float Z;
		float Intensity;
	};
}

void FLidarResultPositionIntensityBarrier::AllocateNative()
{
	check(!HasNative());

	NativeRef->Native = new agxSensor::RtResult();
	NativeRef->Native->add(agxSensor::RtResult::XYZ_VEC3_F32);
	NativeRef->Native->add(agxSensor::RtResult::INTENSITY_F32);
}

void FLidarResultPositionIntensityBarrier::GetResult(
	TArray<FAGX_LidarResultPositionIntensityData>& OutResult) const
{
	using namespace LidarResultPositionIntensityBarrier_helpers;

	check(HasNative());
	AGX_CHECK(sizeof(LidarPositionIntensityData) == GetNative()->Native->getElementSize());

	agxSensor::BinaryResultView<LidarPositionIntensityData> ViewAGX =
		GetNative()->Native->view<LidarPositionIntensityData>();

	OutResult.SetNumUninitialized(ViewAGX.size(), false);
	for (int32 I = 0; I < ViewAGX.size(); I++)
	{
		OutResult[I] = {
			ConvertDisplacement(ViewAGX[I].X, ViewAGX[I].Y, ViewAGX[I].Z), ViewAGX[I].Intensity};
	}
}
