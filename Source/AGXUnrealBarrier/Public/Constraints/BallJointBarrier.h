// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FBallJointBarrier : public FConstraintBarrier
{
public:
	FBallJointBarrier();
	FBallJointBarrier(FBallJointBarrier&& Other) = default;
	FBallJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FBallJointBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier *Rb1, const FVector *FramePosition1, const FQuat *FrameRotation1,
		const FRigidBodyBarrier *Rb2, const FVector *FramePosition2, const FQuat *FrameRotation2) override;

private:
	FBallJointBarrier(const FBallJointBarrier&) = delete;
	void operator=(const FBallJointBarrier&) = delete;
};
