// Fill out your copyright notice in the Description page of Project Settings.

#include "Constraints/AGX_ConstraintFrameActor.h"

#include "EngineUtils.h"

#include "Constraints/AGX_ConstraintFrameComponent.h"

AAGX_ConstraintFrameActor::AAGX_ConstraintFrameActor()
{
	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		ConstraintFrameComponent = CreateDefaultSubobject<UAGX_ConstraintFrameComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		ConstraintFrameComponent->Mobility = EComponentMobility::Movable;
		ConstraintFrameComponent->SetFlags(ConstraintFrameComponent->GetFlags() | RF_Transactional);

#if WITH_EDITORONLY_DATA
		ConstraintFrameComponent->bVisualizeComponent = true;
#endif

		SetRootComponent(ConstraintFrameComponent);
	}
}

void AAGX_ConstraintFrameActor::AddConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	UsedByConstraints.Add(Constraint);
}

void AAGX_ConstraintFrameActor::RemoveConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	// Only remove first occurance, because it is actually valid for a constraint
	// to use the same Constraint Frame Actor twice.
	UsedByConstraints.RemoveSingle(Constraint);
}

const TArray<UAGX_ConstraintComponent*>& AAGX_ConstraintFrameActor::GetConstraintUsage() const
{
	return UsedByConstraints;
}
