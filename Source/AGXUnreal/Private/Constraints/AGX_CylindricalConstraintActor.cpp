#include "Constraints/AGX_CylindricalConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_CylindricalConstraintComponent.h"

AAGX_CylindricalConstraintActor::AAGX_CylindricalConstraintActor()
	: AAGX_Constraint2DofActor(CreateDefaultSubobject<UAGX_CylindricalConstraintComponent>(TEXT("Cylindrical")))
{
}

AAGX_CylindricalConstraintActor::~AAGX_CylindricalConstraintActor()
{
}
