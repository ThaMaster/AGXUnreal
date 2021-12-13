// Copyright 2021, Algoryx Simulation AB.


#include "Terrain/AGX_AsyncLandscapeSampler.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Landscape.h"

FAGX_AsyncLandscapeSampler::FAGX_AsyncLandscapeSampler(
	const ALandscape& InLandscape, const FAGX_LandscapeSizeInfo& InLandscapeSizeInfo,
	const VertexSpan& InSpanX, const VertexSpan& InSpanY)
	: Landscape {InLandscape}
	, LandscapeSizeInfo {InLandscapeSizeInfo}
	, SpanX {InSpanX}
	, SpanY {InSpanY}
{
}

void FAGX_AsyncLandscapeSampler::StartAsync()
{
	if (Thread != nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("StartAsync called with thread already running."));
		return;
	}

	Thread.reset(FRunnableThread::Create(this, TEXT("AsyncLandscapeSampler")));
}

void FAGX_AsyncLandscapeSampler::Join()
{
	if (Thread == nullptr)
	{
		return;
	}

	Thread->WaitForCompletion();
	Thread = nullptr;
}

void FAGX_AsyncLandscapeSampler::Abort()
{
	if (Thread)
	{
		Thread->Kill(true);
		Thread = nullptr;
	}
}

const TArray<float>& FAGX_AsyncLandscapeSampler::GetHeights() const
{
	if (Thread != nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("GetHeights called while thread is running."));
	}

	return Heights;
}

namespace
{
	// Nudge point away from the edge of the landscape if the vertex lies at the edge.
	// By setting ForceNudge = true the point will be nudged even if it does not lie at the
	// landscape edge.
	inline void NudgeEdgePoint(
		const int32 VertX, const int32 VertY, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo,
		float& PointX, float& PointY, bool ForceNudge)
	{
		const float NudgeDistX = LandscapeSizeInfo.QuadSideSizeX / 1000.0f;
		const float NudgeDistY = LandscapeSizeInfo.QuadSideSizeY / 1000.0f;
		const int32 EdgeVertexX = LandscapeSizeInfo.NumVerticesSideX - 1;
		const int32 EdgeVertexY = LandscapeSizeInfo.NumVerticesSideY - 1;

		if (VertX == 0)
		{
			PointX += NudgeDistX;
		}
		else if (VertX == EdgeVertexX || ForceNudge)
		{
			PointX -= NudgeDistX;
		}

		if (VertY == 0)
		{
			PointY += NudgeDistY;
		}
		else if (VertY == EdgeVertexY || ForceNudge)
		{
			PointY -= NudgeDistY;
		}
	}

	// Shoot single ray at landscape to measure the height. Returns false if the ray misses the
	// landscape and true otherwise. If it returns false the OutHeight is set to 0.0 but is not a
	// valid measurement.
	inline bool ShootSingleRay(
		const ALandscape& Landscape, const int32 VertX, const int32 VertY, const float ZOffsetLocal,
		const FAGX_LandscapeSizeInfo& LandscapeSizeInfo,
		const FCollisionQueryParams& CollisionParams, FHitResult& HitResult, float& OutHeight,
		bool ForceNudge = false)
	{
		OutHeight = 0.0f;

		// Vertex position in the landscapes local coordinate system.
		float Xlocal = VertX * LandscapeSizeInfo.QuadSideSizeX;
		float Ylocal = VertY * LandscapeSizeInfo.QuadSideSizeY;

		// The line trace will not detect a hit if it goes through a vertex at the edge of
		// the landscape (might be a floating point precision issue). We fix this by nudging
		// any point at the edge away from the edge and inwards slightly. If the ForceNudge
		// parameter is set to true, the point will be nudged even if it is not on the edge.
		NudgeEdgePoint(VertX, VertY, LandscapeSizeInfo, Xlocal, Ylocal, ForceNudge);

		// Ray start and end positions in global coordinates.
		const FVector RayStart = Landscape.GetTransform().TransformPositionNoScale(
			FVector(Xlocal, Ylocal, ZOffsetLocal));
		const FVector RayEnd = Landscape.GetTransform().TransformPositionNoScale(
			FVector(Xlocal, Ylocal, -ZOffsetLocal));

		if (Landscape.ActorLineTraceSingle(
				HitResult, RayStart, RayEnd, ECC_Visibility, CollisionParams))
		{
			// The hit point of the line trace in the landscape's local coordinate system.
			FVector ImpactPointLocal =
				Landscape.GetTransform().InverseTransformPositionNoScale(HitResult.ImpactPoint);

			OutHeight = ImpactPointLocal.Z;
			return true;
		}

		// Line trace missed.
		return false;
	}
}

uint32 FAGX_AsyncLandscapeSampler::Run()
{
	if (SpanX.End < SpanX.Start || SpanY.End < SpanY.Start)
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid VertexSpan in AsyncLandscapeSampler. The upper bound may not be "
			"lower than the lower bound."));
		return 0;
	}

	const int32 NumVertices = (SpanX.End - SpanX.Start) * (SpanY.End - SpanY.Start);
	check(NumVertices >= 0);

	Heights.Empty();
	Heights.Reserve(NumVertices);

	// At scale = 1, the height span is +- 256 cm
	// https://docs.unrealengine.com/en-US/Engine/Landscape/TechnicalGuide/#calculatingheightmapzscale
	const float HeightSpanHalf = 256.0f * LandscapeSizeInfo.LandscapeScaleZ;

	// Line traces will be used to measure the heights of the landscape.
	const FCollisionQueryParams CollisionParams(FName(TEXT("LandscapeHeightFieldTracess")));
	FHitResult HitResult(ForceInit);
	LineTraceMisses = 0;

	// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
	for (int32 Y = SpanY.End - 1; Y >= SpanY.Start; Y--)
	{
		for (int32 X = SpanX.Start; X < SpanX.End; X++)
		{
			float Height = 0.0f;

			// Use line trace to read the landscape height for this vertex.
			if (ShootSingleRay(
					Landscape, X, Y, HeightSpanHalf, LandscapeSizeInfo, CollisionParams, HitResult,
					Height))
			{
				Heights.Add(Height);
			}

			// Line trace missed. This is unusual but has been observed with large
			// landscapes at the seams between landscape components/sections, similar to
			// line traces at the very edge being missed. Re-try the line trace but force
			// the ray's intersection point to be nudged slightly.
			else if (ShootSingleRay(
						 Landscape, X, Y, HeightSpanHalf, LandscapeSizeInfo, CollisionParams,
						 HitResult, Height, true))
			{
				Heights.Add(Height);
			}
			else
			{
				// Line trace missed even after force nudge, which is unexpected. We will log a
				// warning and set the height value for this vertex to 0.0 and continue. If this
				// happens rarely enough the results may still be useful.
				LineTraceMisses++;
				Heights.Add(0.f);
			}
		}
	}

	check(NumVertices == Heights.Num());

	// Return success;
	return 0;
}
