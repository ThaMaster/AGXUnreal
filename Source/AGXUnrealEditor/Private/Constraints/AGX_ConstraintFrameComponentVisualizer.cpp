#include "Constraints/AGX_ConstraintFrameComponentVisualizer.h"

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponentVisualizer.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"

// Unreal Engine includes.
#include "Editor/UnrealEdEngine.h"
#include "SceneManagement.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintFrameComponentVisualizer"

void FAGX_ConstraintFrameComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintFrameComponent* ConstraintFrameComponent =
		Cast<const UAGX_ConstraintFrameComponent>(Component);

	if (ConstraintFrameComponent == nullptr)
	{
		return;
	}

	const AAGX_ConstraintFrameActor* ConstraintFrameActor =
		Cast<const AAGX_ConstraintFrameActor>(ConstraintFrameComponent->GetOwner());

	if (!ConstraintFrameActor)
	{
		return;
	}

	if (GUnrealEd == nullptr)
	{
		return;
	}

	TSharedPtr<FComponentVisualizer> ComponentVisualizer =
		GUnrealEd->FindComponentVisualizer(UAGX_ConstraintComponent::StaticClass());

	FAGX_ConstraintComponentVisualizer* ConstraintComponentVisualizer =
		static_cast<FAGX_ConstraintComponentVisualizer*>(ComponentVisualizer.Get());

	if (!ConstraintComponentVisualizer)
	{
		return;
	}

	for (UAGX_ConstraintComponent* Constraint : ConstraintFrameActor->GetConstraintUsage())
	{
		ConstraintComponentVisualizer->DrawConstraint(Constraint, View, PDI);
	}
}

#undef LOCTEXT_NAMESPACE
