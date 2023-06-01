// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Plot/PlotBarrier.h"
#include "Plot/AGX_PlotDataSeries.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_PlotComponent.generated.h"

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_PlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_PlotComponent();

	UFUNCTION(BlueprintCallable, Category = "AGX Plot")
	void CreatePlot(
		const FString& Name, UPARAM(ref) FAGX_PlotDataSeries& Xlabel,
		UPARAM(ref) FAGX_PlotDataSeries& Yabel);

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
