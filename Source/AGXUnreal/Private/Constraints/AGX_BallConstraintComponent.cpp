// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_BallConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/ConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

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

void UAGX_BallConstraintComponent::UpdateNativeProperties()
{
	Super::UpdateNativeProperties();

	TwistRangeController.UpdateNativeProperties();
}

FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FBallJointBarrier* UAGX_BallConstraintComponent::GetNativeBallJoint() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_BallConstraintComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
	if (!HasNative())
	{
		return;
	}

	FBallJointBarrier* Barrier = GetNativeBallJoint();
	TwistRangeController.InitializeBarrier(Barrier->GetTwistRangeController());
}
