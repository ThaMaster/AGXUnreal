// Copyright 2022, Algoryx Simulation AB.


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
	NativeBarrier.Reset(new FPrismaticBarrier());
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
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(), GetOwner()->GetFName());
}
