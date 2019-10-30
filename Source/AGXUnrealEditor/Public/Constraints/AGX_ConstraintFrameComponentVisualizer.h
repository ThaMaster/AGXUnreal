// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


/**
 * Component Visualizer of UAGX_ConstraintFrameComponent, which does the following:
 *
 * - Triggers visualization of related Constraint (see FAGX_ConstraintComponentVisualizer).
 *
 */
class AGXUNREALEDITOR_API FAGX_ConstraintFrameComponentVisualizer : public FComponentVisualizer
{
public:

	//~ Begin FComponentVisualizer Interface
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer Interface

};
