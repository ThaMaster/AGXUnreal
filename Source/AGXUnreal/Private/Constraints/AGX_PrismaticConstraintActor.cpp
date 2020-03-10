#include "Constraints/AGX_PrismaticConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_PrismaticConstraintComponent.h"

AAGX_PrismaticConstraintActor::AAGX_PrismaticConstraintActor()
{
	SetConstraintComponent(
		CreateDefaultSubobject<UAGX_PrismaticConstraintComponent>(TEXT("Prismatic")));

}

AAGX_PrismaticConstraintActor::~AAGX_PrismaticConstraintActor()
{
}
