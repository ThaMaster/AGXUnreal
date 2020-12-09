#include "Constraints/AGX_DistanceConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/DistanceJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_DistanceConstraintComponent::UAGX_DistanceConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  TArray<EDofFlag> {
			  // All common DOFs are free.
		  },
		  /*bIsSecondaryConstraintRotational*/ false,
		  /*bIsLockControllerEditable*/ false) // disable because the native impl uses Lock
											   // Controller implicitly!
{
	/// \todo Determine if this is needed, or if the FAGX_ConstraintFrameComponent constructor
	/// does what we want.
	BodyAttachment1.FrameDefiningComponent.Clear();
	BodyAttachment2.FrameDefiningComponent.Clear();
}

UAGX_DistanceConstraintComponent::~UAGX_DistanceConstraintComponent()
{
}

FDistanceJointBarrier* UAGX_DistanceConstraintComponent::GetNativeDistance()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FDistanceJointBarrier* UAGX_DistanceConstraintComponent::GetNativeDistance() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_DistanceConstraintComponent::AllocateNative()
{
	NativeBarrier.Reset(new FDistanceJointBarrier());

	FAGX_ConstraintUtilities::CreateNative(
		GetNative(), BodyAttachment1, BodyAttachment2, GetFName());
}
