#include "Constraints/AGX_HingeConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_HingeConstraintComponent.h"

AAGX_HingeConstraintActor::AAGX_HingeConstraintActor()
	: AAGX_Constraint1DofActor(CreateDefaultSubobject<UAGX_HingeConstraintComponent>(TEXT("Hinge")))
{
}

AAGX_HingeConstraintActor::~AAGX_HingeConstraintActor()
{
}
