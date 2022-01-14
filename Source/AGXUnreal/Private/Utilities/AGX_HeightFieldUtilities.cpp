// Copyright 2022, Algoryx Simulation AB.


#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Terrain/AGX_AsyncLandscapeSampler.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"

#include <limits>

namespace
{
	TArray<float> GetHeights(ALandscape& Landscape, const FAGX_LandscapeSizeInfo& LandscapeSizeInfo)
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
