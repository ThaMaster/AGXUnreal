// Copyright 2021, Algoryx Simulation AB.


#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

#include <memory>

class ALandscape;
struct FAGX_LandscapeSizeInfo;

/**
 * The lifetime of the asynchronous task is bound to the lifetime of this object. Call Join() to
 * ensure that the asynchronous task is completed.
 */
class FAGX_AsyncLandscapeSampler : public FRunnable
{
public:
	struct VertexSpan
	{
		// End is one past the maximum value, so that Vertex < End is always true.
		VertexSpan(int32 InStart, int32 InEnd)
			: Start {InStart}
			, End {InEnd}
		{
			check(InStart < InEnd);
		}

		VertexSpan operator+(int32 Val) const
		{
			return VertexSpan(Start + Val, End + Val);
		}

		VertexSpan operator-(int32 Val) const
		{
			return VertexSpan(Start - Val, End - Val);
		}

		int32 Start;
		int32 End;
	};

	FAGX_AsyncLandscapeSampler(
		const ALandscape& InLandscape, const FAGX_LandscapeSizeInfo& InLandscapeSizeInfo,
		const VertexSpan& InSpanX, const VertexSpan& InSpanY);

	void StartAsync();
	void Join();
	void Abort();

	const TArray<float>& GetHeights() const;

	int32 GetNumLineTraceMisses() const
	{
		return LineTraceMisses;
	}

private:
	virtual uint32 Run() override;

	std::unique_ptr<FRunnableThread> Thread;
	TArray<float> Heights;
	int32 LineTraceMisses = 0;

	const ALandscape& Landscape;
	const FAGX_LandscapeSizeInfo& LandscapeSizeInfo;
	const VertexSpan SpanX;
	const VertexSpan SpanY;
};
