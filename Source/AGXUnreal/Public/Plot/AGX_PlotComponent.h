// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Plot/PlotBarrier.h"
#include "Plot/AGX_PlotDataSeries.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_PlotComponent.generated.h"

/**
 * Component used for Plotting data.
 * Also features data exporting in csv format to file.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlotComponent();

	/**
	 * Crate a new Plot. If the Name is common with any previously created Plot, the curves from
	 * both Plots are placed within the same Plot graph.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	void CreatePlot(
		const FString& Name, UPARAM(ref) FAGX_PlotDataSeries& Xlabel,
		UPARAM(ref) FAGX_PlotDataSeries& Ylabel);

	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	void OpenPlotWindow();

	/**
	 * If set to true, the Plot Window is opened automatically on BeginPlay.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Plot")
	bool bAutoOpenPlotWindow {true};

	UPROPERTY(EditAnywhere, Category = "AGX Plot")
	bool bWriteToFile {true};

	/**
	 * Write data to file in csv format. The file location is the same as the project root.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Plot", Meta = (EditCondition = "bWriteToFile"))
	FString FileOutputName {"AGXUnreal"};

	/// Get the native AGX Dynamics representation of this rigid body. Create it if necessary.
	FPlotBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this rigid body. May return nullptr.
	FPlotBarrier* GetNative();
	const FPlotBarrier* GetNative() const;

	virtual bool HasNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

private:
	void CreateNative();

	// The AGX Dynamics object only exists while simulating. Initialized in
	// BeginPlay and released in EndPlay.
	FPlotBarrier NativeBarrier;
};
