#include "Constraints/AGX_Constraint2DofActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_Constraint2DofComponent.h"

AAGX_Constraint2DofActor::AAGX_Constraint2DofActor()
{
}

AAGX_Constraint2DofActor::~AAGX_Constraint2DofActor()
{
}

AAGX_Constraint2DofActor::AAGX_Constraint2DofActor(
	UAGX_Constraint2DofComponent* InConstraintComponent)
	: AAGX_ConstraintActor(InConstraintComponent)
{
}

UAGX_Constraint2DofComponent* AAGX_Constraint2DofActor::Get2DofComponent()
{
	return Cast<UAGX_Constraint2DofComponent>(ConstraintComponent);
}
