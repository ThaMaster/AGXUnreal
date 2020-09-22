#include "Utilities/AGX_HeightFieldUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"

// AGXUnreal includes.
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

#include <limits>

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

	inline bool ShootSingleRay(
		ALandscape& Landscape, const int32 VertX, const int32 VertY, const float ZOffsetLocal,
		const FAGX_LandscapeSizeInfo& LandscapeSizeInfo,
		const FCollisionQueryParams& CollisionParams, FHitResult& HitResult, float& OutHeight,
		bool ForceNudge = false)
	{
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

	TArray<float> GetHeights(ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
	{
		TArray<float> Heights;
		Heights.AddUninitialized(LandscapeSizeInfo.NumVertices);

		// Line traces will be used to measure the heights of the landscape.
		const FCollisionQueryParams CollisionParams(FName(TEXT("LandscapeHeightFieldTrace")));
		FHitResult HitResult(ForceInit);

		// At scale = 1, the height span is +- 256 cm
		// https://docs.unrealengine.com/en-US/Engine/Landscape/TechnicalGuide/#calculatingheightmapzscale
		const float HEIGHT_SPAN_HALF = 265 * LandscapeSizeInfo.LandscapeScaleZ;

		int32 Vertex = 0;
		int32 LineTraceMisses = 0;
		// AGX terrains Y axis goes in the opposite direction from Unreals Y axis (flipped).
		for (int32 Y = LandscapeSizeInfo.NumVerticesSideY - 1; Y >= 0; Y--)
		{
			for (int32 X = 0; X < LandscapeSizeInfo.NumVerticesSideX; X++)
			{
				float Height = 0.0f;

				// Use line trace to read the landscape height for this vertex.
				if (ShootSingleRay(
						Landscape, X, Y, HEIGHT_SPAN_HALF, LandscapeSizeInfo, CollisionParams,
						HitResult, Height))
				{
					Heights[Vertex] = Height;
				}

				// Line trace missed. This is unusual but has been observed with large
				// landscapes at the seams between landscape components/sections, similar to
				// line traces at the very edge being missed. Re-try the line trace but force
				// the ray's x and y coordinates to be nudged slightly.
				else if (ShootSingleRay(
							 Landscape, X, Y, HEIGHT_SPAN_HALF, LandscapeSizeInfo, CollisionParams,
							 HitResult, Height, true))
				{
					Heights[Vertex] = Height;
				}
				else
				{
					// Line trace missed even after force nudge, which is unexpected. We will log a
					// warning and set the height value for this vertex to 0.0 and continue. If this
					// happens rarely enough the results may still be useful.
					LineTraceMisses++;
					Heights[Vertex] = 0.0;
				}

				Vertex++;
			}
		}

		check(Vertex == LandscapeSizeInfo.NumVertices);

		if (LineTraceMisses > 0)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("%d of %d vertices could not be read from the landscape. The heights of the "
					 "coresponding vertices in the AGX Terrain may therefore be incorrect."),
				LineTraceMisses, Vertex);
		}

		return Heights;
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
