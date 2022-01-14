// Copyright 2021, Algoryx Simulation AB.

#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Terrain/AGX_AsyncLandscapeSampler.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "Misc/Optional.h"

#include <limits>

namespace
{
	TArray<float> GetHeigtsUsingApi(
		ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
	{
		UE_LOG(LogAGX, Warning, TEXT("About to test"));

		const FAGX_AsyncLandscapeSampler::VertexSpan SpanX(0, LandscapeSizeInfo.NumVerticesSideX);
		const FAGX_AsyncLandscapeSampler::VertexSpan SpanY(0, LandscapeSizeInfo.NumVerticesSideY);

		const int32 NumVertices = (SpanX.End - SpanX.Start) * (SpanY.End - SpanY.Start);
		check(NumVertices >= 0);

		TArray<float> Heights;
		Heights.Reserve(NumVertices);

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		for (int32 Y = SpanY.End - 1; Y >= SpanY.Start; Y--)
		{
			for (int32 X = SpanX.Start; X < SpanX.End; X++)
			{
				// Vertex position in the landscapes local coordinate system.
				float Xlocal = static_cast<float>(X) * LandscapeSizeInfo.QuadSideSizeX;
				float Ylocal = static_cast<float>(Y) * LandscapeSizeInfo.QuadSideSizeY;

				if (X == 0)
				{
					Xlocal += 0.1f;
				}
				if (X == SpanX.End - 1)
				{
					Xlocal -= 0.1f;
				}

				if (Y == 0)
				{
					Ylocal += 0.1f;
				}
				if (Y == SpanY.End - 1)
				{
					Ylocal -= 0.1f;
				}

				FVector LocationGlobal =
					Landscape.GetTransform().TransformPositionNoScale(FVector(Xlocal, Ylocal, 0));
				LocationGlobal.Z = 0; // Always set z to zero.

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
						TEXT("Unexpected error: reading height from Landscape at location %f, %f, "
							 "%f failed during AGX Heightfield initialization."),
						LocationGlobal.X, LocationGlobal.Y, LocationGlobal.Z);
					Heights.Add(-300.f);
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
	if (LandsapeRotation.Roll == 0 && LandsapeRotation.Pitch == 0)
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
