#if 0
// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_ConstraintFrame.h"


AAGX_ConstraintFrame::AAGX_ConstraintFrame()
{
	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		Root = CreateDefaultSubobject<USceneComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		Root->Mobility = EComponentMobility::Movable;
		Root->SetFlags(Root->GetFlags() | RF_Transactional);

#if WITH_EDITORONLY_DATA
		Root->bVisualizeComponent = true;
#endif

		SetRootComponent(Root);
	}
}


void AAGX_ConstraintFrame::BeginPlay()
{
	Super::BeginPlay();
}
#endif