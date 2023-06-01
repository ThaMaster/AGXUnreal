// Copyright 2023, Algoryx Simulation AB.

#pragma once

// System includes.
#include <memory>

class FPlotDataSeriesBarrier;
class FSimulationBarrier;

struct FPlotRef;

class AGXUNREALBARRIER_API FPlotBarrier
{
public:
	FPlotBarrier();
	FPlotBarrier(std::unique_ptr<FPlotRef> Native);
	FPlotBarrier(FPlotBarrier&& Other);
	~FPlotBarrier();

	bool HasNative() const;
	FPlotRef* GetNative();
	const FPlotRef* GetNative() const;

	void AllocateNative(
		const FSimulationBarrier& Simulation, const FString* OutputFileName, bool bOpenWebPlot);
	void ReleaseNative();

	void CreatePlot(
		const FString& Name, FPlotDataSeriesBarrier& Xlabel, FPlotDataSeriesBarrier& Ylabel);

	void OpenWebPlot();

private:
	FPlotBarrier(const FPlotBarrier&) = delete;
	void operator=(const FPlotBarrier&) = delete;

private:
	std::unique_ptr<FPlotRef> NativeRef;
};
