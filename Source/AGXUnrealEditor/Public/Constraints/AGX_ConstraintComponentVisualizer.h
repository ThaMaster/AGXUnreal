// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


/**
 * Component Visualizer of UAGX_ConstraintComponent, which does the following:
 *
 * - Highlights involved Rigid Body Actors.
 * - Draw tripods for the final Attachment Frames.
 *
 */
class AGXUNREALEDITOR_API FAGX_ConstraintComponentVisualizer : public FComponentVisualizer
{
public:

	//~ Begin FComponentVisualizer Interface
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer Interface

private:

	void DrawConstraint(const AAGX_Constraint* Constraint, const FSceneView* View, FPrimitiveDrawInterface* PDI);
};
