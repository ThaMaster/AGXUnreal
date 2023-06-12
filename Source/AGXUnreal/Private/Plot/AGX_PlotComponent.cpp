// Copyright 2023, Algoryx Simulation AB.

#include "Plot/AGX_PlotComponent.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_StringUtilities.h"

UAGX_PlotComponent::UAGX_PlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_PlotComponent::CreatePlot(
	const FString& Name, FAGX_PlotDataSeries& SeriesX, FAGX_PlotDataSeries& SeriesY)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreatePlot was called on Plot '%s' in '%s' but the Plot does not have a AGX "
				 "Native. This function should only be called during Play."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	// The reason we allocate the PlotDataSeries Natives here is that they are of UStruct type,
	// meaning they do not get the BeginPlay callback where AllocateNative is usually called.
	if (!SeriesX.HasNative())
		SeriesX.NativeBarrier.AllocateNative(SeriesX.Label);

	if (!SeriesY.HasNative())
		SeriesY.NativeBarrier.AllocateNative(SeriesY.Label);

	NativeBarrier.CreatePlot(Name, SeriesX.NativeBarrier, SeriesY.NativeBarrier);
}

void UAGX_PlotComponent::OpenPlotWindow()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("OpenPlotWindow was called on Plot '%s' in '%s' but the Plot does not have a AGX "
				 "Native. This function should only be called during Play."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	NativeBarrier.OpenWebPlot();
}

FPlotBarrier* UAGX_PlotComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Plot '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	check(HasNative());
	return &NativeBarrier;
}

FPlotBarrier* UAGX_PlotComponent::GetNative()
{
	return &NativeBarrier;
}

const FPlotBarrier* UAGX_PlotComponent::GetNative() const
{
	return &NativeBarrier;
}

bool UAGX_PlotComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

void UAGX_PlotComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateNative();
}

void UAGX_PlotComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_PlotComponent::CreateNative()
{
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Plot '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	if (!Simulation->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Plot '%s' in '%s' tried to get Simulation Native, but UAGX_Simulation::HasNative "
				 "returned false."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FString* OutFile = bWriteToFile && !FileOutputName.IsEmpty() ? &FileOutputName : nullptr;
	const bool AutoOpenWindow = bAutoOpenPlotWindow && !GIsReconstructingBlueprintInstances;
	NativeBarrier.AllocateNative(*Simulation->GetNative(), OutFile, AutoOpenWindow);
}
