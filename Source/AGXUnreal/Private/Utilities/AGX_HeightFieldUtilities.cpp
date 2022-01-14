// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Terrain/AGX_AsyncLandscapeSampler.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"
#include "Math/UnrealMathUtility.h"

#include <limits>

namespace
{
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
				// Vertex position in the landscapes local coordinate system.
				const float Xlocal = [&]()
				{
					float Xl = static_cast<float>(X) * LandscapeSizeInfo.QuadSideSizeX;
					// Measurements right at the edge of the landscape fails for some reason. Nudge
					// the measurement point slightly towards center at edges as a workaround.
					if (X == 0)
					{
						Xl += EdgeNudgeDistanceX;
					}
					else if (X == LastVertIndexX)
					{
						Xl -= EdgeNudgeDistanceX;
					}
					return Xl;
				}();

				const float Ylocal = [&]()
				{
					float Yl = static_cast<float>(Y) * LandscapeSizeInfo.QuadSideSizeY;
					// Measurements right at the edge of the landscape fails for some reason. Nudge
					// the measurement point slightly towards center at edges as a workaround.
					if (Y == 0)
					{
						Yl += EdgeNudgeDistanceY;
					}
					else if (Y == LastVertIndexY)
					{
						Yl -= EdgeNudgeDistanceY;
					}
					return Yl;
				}();

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

	TArray<float> GetHeightsUsingRayCasts(
		ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
	{
		const int32 NumThreads =
			FMath::Clamp(FPlatformMisc::NumberOfCores() - 1, 1, LandscapeSizeInfo.NumVerticesSideY);

		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights with ray casting using %d "
			"threads."), NumThreads);

		// Split the Landscape up between the threads so that each thread gets an equal range of
		// Y-side vertices, except for the last thread that also gets any remainder from the
		// division. This way of splitting up the Landscape ensures that the height data is stored
		// in the correct order when finally concatenating the TArrays that the threads produced.
		const int32 YVertsPerThread =
			LandscapeSizeInfo.NumVerticesSideY / NumThreads; // Truncates down to nearest integer.
		const FAGX_AsyncLandscapeSampler::VertexSpan SpanX(0, LandscapeSizeInfo.NumVerticesSideX);
		FAGX_AsyncLandscapeSampler::VertexSpan SpanY(
			LandscapeSizeInfo.NumVerticesSideY - YVertsPerThread,
			LandscapeSizeInfo.NumVerticesSideY);

		TArray<FAGX_AsyncLandscapeSampler> Samplers;
		for (int32 i = 0; i < NumThreads; i++)
		{
			if (i == NumThreads - 1)
			{
				// The last Sampler gets any remainder from the division of Y-side vertices among
				// the threads, i.e. it's SpanY always goes from 0.
				Samplers.Add(FAGX_AsyncLandscapeSampler(
					Landscape, LandscapeSizeInfo, SpanX, {0, SpanY.End}));
			}
			else
			{
				Samplers.Add(
					FAGX_AsyncLandscapeSampler(Landscape, LandscapeSizeInfo, SpanX, SpanY));
			}

			SpanY = SpanY - YVertsPerThread;
		}

		for (auto& Sampler : Samplers)
		{
			Sampler.StartAsync();
		}

		TArray<float> Heights;
		Heights.Reserve(LandscapeSizeInfo.NumVertices);
		int32 LineTraceMisses = 0;
		for (auto& Sampler : Samplers)
		{
			Sampler.Join();
			Heights.Append(Sampler.GetHeights());
			LineTraceMisses += Sampler.GetNumLineTraceMisses();
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
