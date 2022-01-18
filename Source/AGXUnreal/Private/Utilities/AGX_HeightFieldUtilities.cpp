// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"
#include "Math/UnrealMathUtility.h"

// Standard library includes.
#include <limits>

namespace AGX_HeightFieldUtilities_helpers
{
	// Nudge point away from the edge of the landscape if the vertex lies at the edge.
	// By setting ForceNudge = true the point will be nudged even if it does not lie at the
	// landscape edge.
	void NudgeEdgePoint(
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
	// landscape and true otherwise. If it returns false the OutHeight is set to 0.0 but is not
	// a valid measurement.
	bool ShootSingleRay(
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

	// This function should only be used if the landscape is not rotated around world x or y axis.
	// The reason for this is that the Landscape.GetHeightAtLocaion does not handle that case. It
	// will measure along the world z-axis (instead of the Landscapes local z-axis as it should)
	// such that sharp peaks will be cut off and tilted.
	TArray<float> GetHeigtsUsingApi(
		ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
	{
		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights using Landscape API."));

		TArray<float> Heights;
		const int32 NumVertices =
			LandscapeSizeInfo.NumVerticesSideX * LandscapeSizeInfo.NumVerticesSideY;
		if (NumVertices <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetHeightsUsingAPI got zero sized landscape. Cannot read landscape heights "
					 "from landscape '%s'."),
				*Landscape.GetName());
		}

		Heights.Reserve(NumVertices);
		const int32 LastVertIndexX = LandscapeSizeInfo.NumVerticesSideX - 1;
		const int32 LastVertIndexY = LandscapeSizeInfo.NumVerticesSideY - 1;
		const float EdgeNudgeDistanceX = LandscapeSizeInfo.QuadSideSizeX / 1000.0f;
		const float EdgeNudgeDistanceY = LandscapeSizeInfo.QuadSideSizeY / 1000.0f;

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		for (int32 Y = LastVertIndexY; Y >= 0; Y--)
		{
			for (int32 X = 0; X <= LastVertIndexX; X++)
			{
				float Xlocal = static_cast<float>(X) * LandscapeSizeInfo.QuadSideSizeX;
				float Ylocal = static_cast<float>(Y) * LandscapeSizeInfo.QuadSideSizeY;

				// Measurements right at the edge of the landscape fails for some reason. Nudge
				// the measurement point slightly towards center at edges as a workaround.
				NudgeEdgePoint(X, Y, LandscapeSizeInfo, Xlocal, Ylocal, false);

				FVector LocationGlobal =
					Landscape.GetTransform().TransformPositionNoScale(FVector(Xlocal, Ylocal, 0));
				TOptional<float> Height = Landscape.GetHeightAtLocation(LocationGlobal);
				if (Height.IsSet())
				{
					// Position of height measurement in Landscapes local coordinate system.
					FVector HeightPointLocal =
						Landscape.GetTransform().InverseTransformPositionNoScale(
							FVector(LocationGlobal.X, LocationGlobal.Y, *Height));
					Heights.Add(HeightPointLocal.Z);
				}
				else
				{
					UE_LOG(
						LogAGX, Error,
						TEXT("Unexpected error: reading height from Landscape '%s' at location %f, "
							 "%f, %f failed during AGX Heightfield initialization."),
						*Landscape.GetName(), LocationGlobal.X, LocationGlobal.Y, LocationGlobal.Z);
					Heights.Add(0.f);
				}
			}
		}

		check(NumVertices == Heights.Num());

		return Heights;
	}

	// This is an alternative to AGX_HeightFieldUtilities_helpers::GetHeigtsUsingApi. This function
	// is slower but can handle any Landscape orientation, which is not the case for
	// AGX_HeightFieldUtilities_helpers::GetHeigtsUsingApi (see comment above that function).
	TArray<float> GetHeightsUsingRayCasts(
		ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
	{
		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights with ray casting."));
		TArray<float> Heights;
		Heights.Reserve(LandscapeSizeInfo.NumVertices);
		int32 LineTraceMisses = 0;

		// At scale = 1, the height span is +- 256 cm
		// https://docs.unrealengine.com/en-US/Engine/Landscape/TechnicalGuide/#calculatingheightmapzscale
		const float HeightSpanHalf = 256.0f * LandscapeSizeInfo.LandscapeScaleZ;

		// Line traces will be used to measure the heights of the landscape.
		const FCollisionQueryParams CollisionParams(FName(TEXT("LandscapeHeightFieldTracess")));
		FHitResult HitResult(ForceInit);

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		for (int32 Y = LandscapeSizeInfo.NumVerticesSideY - 1; Y >= 0; Y--)
		{
			for (int32 X = 0; X < LandscapeSizeInfo.NumVerticesSideX; X++)
			{
				float Height = 0.0f;

				// Use line trace to read the landscape height for this vertex.
				if (ShootSingleRay(
						Landscape, X, Y, HeightSpanHalf, LandscapeSizeInfo, CollisionParams,
						HitResult, Height))
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


		check(Heights.Num() == LandscapeSizeInfo.NumVertices);
		if (LineTraceMisses > 0)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("%d of %d vertices could not be read from the landscape. The heights of the "
					 "coresponding vertices in the AGX Terrain may therefore be incorrect."),
				LineTraceMisses, LandscapeSizeInfo.NumVertices);
		}

		return Heights;
	}
}

TArray<float> GetHeights(ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
{
	using namespace AGX_HeightFieldUtilities_helpers;
	const FRotator LandsapeRotation = Landscape.GetActorRotation();
	if (FMath::IsNearlyZero(LandsapeRotation.Roll, KINDA_SMALL_NUMBER) &&
		FMath::IsNearlyZero(LandsapeRotation.Pitch, KINDA_SMALL_NUMBER))
	{
		// If the Landscape is not rotated around x or y, we can use the Landscape API to read the
		// heights which is much faster than ray-casting.
		return GetHeigtsUsingApi(Landscape, LandscapeSizeInfo);
	}
	else
	{
		return GetHeightsUsingRayCasts(Landscape, LandscapeSizeInfo);
	}
}

FHeightFieldShapeBarrier AGX_HeightFieldUtilities::CreateHeightField(ALandscape& Landscape)
{
	const FAGX_LandscapeSizeInfo LandscapeSizeInfo(Landscape);

	if (LandscapeSizeInfo.NumComponents <= 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_HeightFieldUtilities::CreateHeightField cannot create heightfield "
				 "from landscape without components."));

		// Return empty FHeightFieldShapeBarrier (no native allocated).
		return FHeightFieldShapeBarrier();
	}

	TArray<float> Heights = GetHeights(Landscape, LandscapeSizeInfo);
	const float SideSizeX = LandscapeSizeInfo.NumQuadsSideX * LandscapeSizeInfo.QuadSideSizeX;
	const float SideSizeY = LandscapeSizeInfo.NumQuadsSideY * LandscapeSizeInfo.QuadSideSizeY;

	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(
		LandscapeSizeInfo.NumVerticesSideX, LandscapeSizeInfo.NumVerticesSideY, SideSizeX,
		SideSizeY, Heights);

	return HeightField;
}
