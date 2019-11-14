// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AGX_ConstraintComponent.generated.h"


/**
 * Component owned by every Constraint Actor so that component features can be used.
 * For example, enables the usage of a Component Visualizer, so that helpful graphics
 * can be shown in the Level Editor Viewport when editing the constraint.
 *
 * @see FAGX_ConstraintComponentVisualizer
 *
 */
UCLASS(Category = "AGX", ClassGroup= "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	
	UAGX_ConstraintComponent();
		
};
