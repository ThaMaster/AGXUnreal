#include "Constraints/AGX_BallConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_BallConstraintComponent.h"

AAGX_BallConstraintActor::AAGX_BallConstraintActor()
	: AAGX_ConstraintActor(CreateDefaultSubobject<UAGX_BallConstraintComponent>(TEXT("Ball")))
{
}

AAGX_BallConstraintActor::~AAGX_BallConstraintActor()
{
}
