// Copyright 2023, Algoryx Simulation AB.

#include "Plot/PlotBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "Plot/PlotDataSeriesBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxPlot/FilePlot.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/Paths.h"


FPlotBarrier::FPlotBarrier()
	: NativeRef {new FPlotRef}
{
}

FPlotBarrier::FPlotBarrier(std::unique_ptr<FPlotRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FPlotBarrier::FPlotBarrier(FPlotBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FPlotRef);
}

FPlotBarrier::~FPlotBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FPlotRef.
}

bool FPlotBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FPlotBarrier::AllocateNative(
	const FSimulationBarrier& Simulation, const FString* OutputFileName, bool bOpenWebPlot)
{
	check(!HasNative());
	check(Simulation.HasNative());
	NativeRef->Native = Simulation.GetNative()->Native->getPlotSystem();

	if (bOpenWebPlot)
	{
		OpenWebPlot();
	}

	if (OutputFileName == nullptr)
	{
		return; // We are done.
	}

	const FString RelOutputDir = FPaths::GetPath(FPaths::GetProjectFilePath());
	const FString OutputDir =
		IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*RelOutputDir);
	if (!FPaths::DirectoryExists(OutputDir))
	{
		// Todo: log via notification here.
	}
	else
	{
		const FString OutputPath = FPaths::Combine(OutputDir, *OutputFileName + FString(".dat"));
		NativeRef->Native->add(new agxPlot::FilePlot(Convert(OutputPath)));
	}
}

FPlotRef* FPlotBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FPlotRef* FPlotBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FPlotBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

void FPlotBarrier::CreatePlot(
	const FString& Name, FPlotDataSeriesBarrier& Xlabel, FPlotDataSeriesBarrier& Ylabel,
	TArray<FPlotDataSeriesBarrier*>& YlabelsBarriers)
{
	check(HasNative());
	check(Xlabel.HasNative());
	check(Ylabel.HasNative());

	agxPlot::Window* plotWindow = NativeRef->Native->getOrCreateWindow(Convert(Name));

	agxPlot::DataSeries* X = Xlabel.GetNative()->Native; 
	agxPlot::DataSeries* Y = Ylabel.GetNative()->Native; 
	plotWindow->add(new agxPlot::Curve(X, Y, Y->getName()));

	/*for (auto& PDSB : YlabelsBarriers)
	{
		agxPlot::DataSeries* DS = PDSB.GetNative()->Native;
		plotWindow->add(new agxPlot::Curve(X, DS, DS->getName()));
	}*/
}

void FPlotBarrier::OpenWebPlot()
{
	check(HasNative());
	NativeRef->Native->add(new agxPlot::WebPlot(true));
}
