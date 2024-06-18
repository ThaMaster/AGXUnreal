// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_BallConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

// Special member functions.

UAGX_BallConstraintComponent::UAGX_BallConstraintComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3})
{
	NativeBarrier.Reset(new FBallJointBarrier());
}

UAGX_BallConstraintComponent::~UAGX_BallConstraintComponent()
{
}

// Function overrides.

//~ Begin AGX Constraint Component interface.

void UAGX_BallConstraintComponent::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();
#if 1
	TwistRangeController.UpdateNativeProperties();
#endif
}

//~ End AGX Constraint Component interface.


// Native management.

FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

#if 1
namespace AGX_BallConstraintComponent_helpers
{
	void InitializeControllerBarriers(UAGX_BallConstraintComponent& Constraint)
	{
		FBallJointBarrier* Barrier = Constraint.GetNativeBallJoint();
		Constraint.TwistRangeController.InitializeBarrier(Barrier->GetTwistRangeController());
	}
}
#endif

void UAGX_BallConstraintComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
	if (!HasNative())
	{
		return;
	}

#if 1
	AGX_BallConstraintComponent_helpers::InitializeControllerBarriers(*this);
#endif
}
