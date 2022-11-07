// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Math/UnrealMathUtility.h"

// Standard library includes.
#include <limits>

namespace AGX_HeightFieldUtilities_helpers
{

	FTransform GetAGXTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent,
		bool IsTerrain)
	{
		const FVector CenterProjectedLocal = [&]()
		{
			FVector CenterLocal =
				Landscape.GetActorTransform().InverseTransformPositionNoScale(Center);
			CenterLocal.Z = 0.0;
			return CenterLocal;
		}();

		if (!IsTerrain)
		{
			return FTransform(
				Landscape.GetActorQuat(),
				Landscape.GetActorTransform().TransformPositionNoScale(CenterProjectedLocal));
		}

		// The terrain will be offset half a tile if the number of tiles are odd. This is an AGX
		// Dynamics thing.
		const auto QuadSideSizeX = Landscape.GetActorScale().X;
		const auto QuadSideSizeY = Landscape.GetActorScale().Y;
		const int32 NumQuadsX = FMath::RoundToInt32(2.0 * HalfExtent.X / QuadSideSizeX);
		const int32 NumQuadsY = FMath::RoundToInt32(2.0 * HalfExtent.Y / QuadSideSizeY);
		const float TerrainTileCenterOffsetX = (NumQuadsX % 2 == 0) ? 0 : QuadSideSizeX / 2;
		const float TerrainTileCenterOffsetY = (NumQuadsY % 2 == 0) ? 0 : -QuadSideSizeY / 2;
		FVector LocalTileOffset(TerrainTileCenterOffsetX, TerrainTileCenterOffsetY, 0);

		const FVector CenterProjectedGlobalAdjusted =
			Landscape.GetActorTransform().TransformPositionNoScale(
				CenterProjectedLocal + LocalTileOffset);
		return FTransform(Landscape.GetActorQuat(), CenterProjectedGlobalAdjusted);
	}

	// Nudge point away from the edge of the landscape if the vertex lies at the edge.
	// By setting ForceNudge = true the point will be nudged even if it does not lie at the
	// landscape edge.
	/* void NudgeEdgePoint(
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
	}*/

	void NudgePoint(float& X, float& Y, float MinX, float MaxX, float MinY, float MaxY)
	{
		static constexpr float NudgeDist = 0.1f;
		if (X <= MinX)
			X += NudgeDist;
		else if (X >= MaxX)
			X -= NudgeDist;
		if (Y <= MinY)
			Y += NudgeDist;
		else if (Y >= MaxY)
			Y -= NudgeDist;
	}
#if 0
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
//		NudgeEdgePoint(VertX, VertY, LandscapeSizeInfo, Xlocal, Ylocal, ForceNudge);

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
#endif
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
		const float MaxX = StartPosLocal.X + LengthX;
		const float MaxY = StartPosLocal.Y + LengthY;
		float CurrentX = StartPosLocal.X;
		float CurrentY = StartPosLocal.Y + LengthY;
		static constexpr float Tolerance = 0.2;
		while (CurrentY >= StartPosLocal.Y - Tolerance)
		{
			while (CurrentX <= MaxX + Tolerance)
			{
				FVector LocationGlobal = Landscape.GetTransform().TransformPositionNoScale(
					FVector(CurrentX, CurrentY, 0));
				TOptional<float> Height = Landscape.GetHeightAtLocation(LocationGlobal);
				if (!Height.IsSet())
				{
					// Attempt to nudge the measuring point a little and do the measurement again.
					// We do this because sometimes, measuring at the edge of a Landscape does not
					// work.
					float NudgedX = CurrentX;
					float NudgedY = CurrentY;
					NudgePoint(NudgedX, NudgedY, StartPosLocal.X, MaxX, StartPosLocal.Y, MaxY);

					LocationGlobal = Landscape.GetTransform().TransformPositionNoScale(
						FVector(NudgedX, NudgedY, 0));
					Height = Landscape.GetHeightAtLocation(LocationGlobal);
				}
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
#if 0
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
#endif
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

FTransform AGX_HeightFieldUtilities::GetTerrainTransformUsingBoxFrom(
	const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformUsingBoxFrom(
		Landscape, Center, HalfExtent, true);
}

FTransform AGX_HeightFieldUtilities::GetHeightFieldTransformUsingBoxFrom(
	const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformUsingBoxFrom(
		Landscape, Center, HalfExtent, false);
}

std::tuple<int32, int32> AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(
	const ALandscape& Landscape)
{
	float SizeX, SizeY;
	std::tie(SizeX, SizeY) = GetLandscapeSizeXY(Landscape);
	const auto QuadSideSizeX = Landscape.GetActorScale().X;
	const auto QuadSideSizeY = Landscape.GetActorScale().Y;
	return std::make_tuple<int32, int32>(
		FMath::RoundToInt32(SizeX / QuadSideSizeX) + 1,
		FMath::RoundToInt32(SizeY / QuadSideSizeY) + 1);
}

std::tuple<float, float> AGX_HeightFieldUtilities::GetLandscapeSizeXY(const ALandscape& Landscape)
{
	const ALandscapeProxy* LandscapeProxy = Cast<ALandscapeProxy>(&Landscape);
	const ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
	FIntRect Rect;
	LandscapeInfo->GetLandscapeExtent(Rect.Min.X, Rect.Min.Y, Rect.Max.X, Rect.Max.Y);
	FIntPoint Size = Rect.Size();

	// VertsXY here is the vertex count of the smallest bounded region of the Landscape.
	// So if Landscape Components has been removed using the Landscape tool, this will not
	// necessarily reflect the overall original landscape vertex count.
	// We do a little trick using this vertex count and the ActorBounds origin below which
	// we know lies in the center of the smallest bounded region of the Landscape.
	// @todo Figure out how to get the original landscape size properly. This will not handle
	// the case where a complete outer side-slice has been removed from the landscape along the
	// Y-axis.
	const auto VertsXY = std::make_tuple<int32, int32>(Size.X + 1, Size.Y + 1);
	const auto QuadSideSizeX = Landscape.GetActorScale().X;
	const auto QuadSideSizeY = Landscape.GetActorScale().Y;

	FVector ActorOrigin, Unused;
	LandscapeProxy->GetActorBounds(false, ActorOrigin, Unused);

	const FVector ActorOriginLocal =
		Landscape.GetActorTransform().InverseTransformPositionNoScale(ActorOrigin);

	const auto SizeX = static_cast<float>(std::get<0>(VertsXY) - 1) * QuadSideSizeX / 2.0 +
					   FMath::Abs(ActorOriginLocal.X);
	const auto SizeY = static_cast<float>(std::get<1>(VertsXY) - 1) * QuadSideSizeY / 2.0 +
					   FMath::Abs(ActorOriginLocal.Y);
	return std::make_tuple<float, float>(SizeX, SizeY);
}

bool AGX_HeightFieldUtilities::IsOpenWorldLandscape(const ALandscape& Landscape)
{
	// This is just an observation that holds true for OpenWorldLandscapes, would be better
	// with a more "correct" way of determining this.
	return Landscape.LandscapeComponents.Num() == 0;
}