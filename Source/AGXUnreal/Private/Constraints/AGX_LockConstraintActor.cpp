#include "Constraints/AGX_LockConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_LockConstraintComponent.h"

AAGX_LockConstraintActor::AAGX_LockConstraintActor()
{
	SetConstraintComponent(CreateDefaultSubobject<UAGX_LockConstraintComponent>(TEXT("Lock")));

}

AAGX_LockConstraintActor::~AAGX_LockConstraintActor()
{
}
