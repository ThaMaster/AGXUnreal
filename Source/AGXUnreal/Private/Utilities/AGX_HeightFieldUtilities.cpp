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

	std::tuple<FVector, FQuat> GetAGXTransformFrom(const ALandscape& Landscape, bool IsTerrain)
	{
		// An Unreal landscape has its origin in the bottom left corner.
		// A AGX Dynamics Terrain (and Geometry having a Hight Field Shape) has their origin
		// at the center.
		const FAGX_LandscapeSizeInfo LandscapeSizeInfo(Landscape);
		const float SideSizeX = LandscapeSizeInfo.NumQuadsSideX * LandscapeSizeInfo.QuadSideSizeX;
		const float SideSizeY = LandscapeSizeInfo.NumQuadsSideY * LandscapeSizeInfo.QuadSideSizeY;

		const FVector LandscapeToCenterOffsetLocal = [&]()
		{
			if (IsTerrain)
			{
				// For a AGX Dynamics Terrain; if there are an odd number of tiles in the
				// x-direction, the origins x-coordinate is the same as the x-coordinate of the left
				// edge of the center tile. If there are an odd number of tiles in the y-direction,
				// the origins y-coordinate is the same as the y-coordinate of the top edge of the
				// center tile.
				const float TerrainTileCenterOffsetX = (LandscapeSizeInfo.NumQuadsSideX % 2 == 0)
														   ? 0
														   : LandscapeSizeInfo.QuadSideSizeX / 2;
				const float TerrainTileCenterOffsetY = (LandscapeSizeInfo.NumQuadsSideY % 2 == 0)
														   ? 0
														   : -LandscapeSizeInfo.QuadSideSizeY / 2;
				return FVector(
					SideSizeX / 2.0f + TerrainTileCenterOffsetX,
					SideSizeY / 2.0f + TerrainTileCenterOffsetY, 0);
			}
			else
			{
				return FVector(SideSizeX / 2.0f, SideSizeY / 2.0f, 0);
			}
		}();

		// Transform the offset from landscape local coordinate system to the global coordinate
		// system.
		const FTransform LandscapeTransform = Landscape.GetTransform();
		const FVector WorldLocation =
			LandscapeTransform.TransformPositionNoScale(LandscapeToCenterOffsetLocal);
		return std::make_tuple(WorldLocation, Landscape.GetActorQuat());
	}

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
		ALandscape& Landscape, const FVector& StartPos, float LengthX, float LengthY)
	{
		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights using Landscape API."));

		const auto QuadSideSizeX = Landscape.GetActorScale().X;
		const auto QuadSideSizeY = Landscape.GetActorScale().Y;
		const int32 ResolutionX = FMath::RoundToInt32(LengthX / QuadSideSizeX);
		const int32 ResolutionY = FMath::RoundToInt32(LengthY / QuadSideSizeY);
		const int32 VerticesSideX = ResolutionX + 1;
		const int32 VerticesSideY = ResolutionY + 1;

		TArray<float> Heights;
		const int32 NumVertices = VerticesSideX * VerticesSideY;
		if (NumVertices <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetHeightsUsingAPI got zero sized landscape. Cannot read landscape heights "
					 "from landscape '%s'."),
				*Landscape.GetName());
			return Heights;
		}

		Heights.Reserve(NumVertices);
		const float EdgeNudgeDistanceX = QuadSideSizeX / 1000.0f;
		const float EdgeNudgeDistanceY = QuadSideSizeY / 1000.0f;
		const FVector StartPosLocal =
			Landscape.GetActorTransform().InverseTransformPositionNoScale(StartPos);

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		const float XMax = StartPosLocal.X + LengthX;
		float CurrentX = StartPosLocal.X; 
		float CurrentY = StartPosLocal.Y + LengthY; 
		static constexpr float Tolerance = 0.5;
		while (CurrentY >= StartPosLocal.Y - Tolerance)
		{
			while (CurrentX <= XMax + Tolerance)
			{
				FVector LocationGlobal = Landscape.GetTransform().TransformPositionNoScale(
					FVector(CurrentX, CurrentY, 0));
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
				CurrentX += QuadSideSizeX;
			}
			CurrentX = StartPosLocal.X;
			CurrentY -= QuadSideSizeY;
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

TArray<float> GetHeights(
	ALandscape& Landscape, const FVector& StartPos, float LengthX, float LengthY)
{
	using namespace AGX_HeightFieldUtilities_helpers;
	const FRotator LandsapeRotation = Landscape.GetActorRotation();
	if (FMath::IsNearlyZero(LandsapeRotation.Roll, KINDA_SMALL_NUMBER) &&
		FMath::IsNearlyZero(LandsapeRotation.Pitch, KINDA_SMALL_NUMBER))
	{
		// If the Landscape is not rotated around x or y, we can use the Landscape API to read the
		// heights which is much faster than ray-casting.
		return GetHeigtsUsingApi(Landscape, StartPos, LengthX, LengthY);
	}
	else
	{
		return TArray<float>();
		// return GetHeightsUsingRayCasts(Landscape, LandscapeSizeInfo);
	}
}

FHeightFieldShapeBarrier AGX_HeightFieldUtilities::CreateHeightField(
	ALandscape& Landscape, const FVector& StartPos, float LengthX, float LengthY)
{
	TArray<float> Heights = GetHeights(Landscape, StartPos, LengthX, LengthY);
	const auto QuadSideSizeX = Landscape.GetActorScale().X;
	const auto QuadSideSizeY = Landscape.GetActorScale().Y;
	const int32 ResolutionX = FMath::RoundToInt32(LengthX / QuadSideSizeX) + 1;
	const int32 ResolutionY = FMath::RoundToInt32(LengthY / QuadSideSizeY) + 1;
	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(
		ResolutionX, ResolutionY, FMath::RoundToFloat(LengthX), FMath::RoundToFloat(LengthY),
		Heights);

	return HeightField;
}

std::tuple<FVector, FQuat> AGX_HeightFieldUtilities::GetTerrainPositionAndRotationFrom(
	const ALandscape& Landscape)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformFrom(Landscape, true);
}

std::tuple<FVector, FQuat> AGX_HeightFieldUtilities::GetHeightFieldPositionAndRotationFrom(
	const ALandscape& Landscape)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformFrom(Landscape, false);
}