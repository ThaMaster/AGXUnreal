// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "Constraints/ConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#if 1
#include "Constraints/ControllerConstraintBarriers.h"
#endif


class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FBallJointBarrier : public FConstraintBarrier
{
public: // Special member functions.
	FBallJointBarrier();
	FBallJointBarrier(FBallJointBarrier&& Other) = default;
	FBallJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FBallJointBarrier();

public:
#if 1
	FTwistRangeControllerBarrier GetTwistRangeController() const;
#endif

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FBallJointBarrier(const FBallJointBarrier&) = delete;
	void operator=(const FBallJointBarrier&) = delete;
};
