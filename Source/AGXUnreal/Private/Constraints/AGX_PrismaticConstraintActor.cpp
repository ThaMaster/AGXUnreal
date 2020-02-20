#include "Constraints/AGX_PrismaticConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_PrismaticConstraintComponent.h"

AAGX_PrismaticConstraintActor::AAGX_PrismaticConstraintActor()
	: AAGX_Constraint1DofActor(
		  CreateDefaultSubobject<UAGX_PrismaticConstraintComponent>(TEXT("Prismatic")))
{
}

AAGX_PrismaticConstraintActor::~AAGX_PrismaticConstraintActor()
{
}
