// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_ConstraintFrame.h"


AAGX_ConstraintFrame::AAGX_ConstraintFrame()
{
	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		USceneComponent *NewComponent = CreateDefaultSubobject<USceneComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		NewComponent->Mobility = EComponentMobility::Movable;
		NewComponent->SetFlags(NewComponent->GetFlags() | RF_Transactional);

#if WITH_EDITORONLY_DATA
		NewComponent->bVisualizeComponent = true;
#endif

		SetRootComponent(NewComponent);
	}
}


void AAGX_ConstraintFrame::BeginPlay()
{
	Super::BeginPlay();
}
