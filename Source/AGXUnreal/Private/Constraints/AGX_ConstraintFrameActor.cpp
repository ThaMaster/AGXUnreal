// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/AGX_ConstraintFrameActor.h"

#include "EngineUtils.h"


AAGX_ConstraintFrameActor::AAGX_ConstraintFrameActor()
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


void AAGX_ConstraintFrameActor::AddConstraintUsage(AAGX_Constraint* Constraint)
{
	UsedByConstraints.Add(Constraint);
}


void AAGX_ConstraintFrameActor::RemoveConstraintUsage(AAGX_Constraint* Constraint)
{
	// Only remove first occurance, because it is actually valid for a constraint
	// to use the same Constraint Frame Actor twice.
	UsedByConstraints.RemoveSingle(Constraint);
}
