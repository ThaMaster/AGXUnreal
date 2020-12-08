#include "Constraints/AGX_PrismaticConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/PrismaticBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_PrismaticConstraintComponent::UAGX_PrismaticConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DofFlagRotational1, EDofFlag::DofFlagRotational2, EDofFlag::DofFlagRotational3,
		   EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2},
		  /*bIsSecondaryConstraintRotational*/ false)
{
}

UAGX_PrismaticConstraintComponent::~UAGX_PrismaticConstraintComponent()
{
}

FPrismaticBarrier* UAGX_PrismaticConstraintComponent::GetNativePrismatic()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FPrismaticBarrier* UAGX_PrismaticConstraintComponent::GetNativePrismatic() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_PrismaticConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FPrismaticBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
