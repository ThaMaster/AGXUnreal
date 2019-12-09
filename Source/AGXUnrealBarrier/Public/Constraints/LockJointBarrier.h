// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/ConstraintBarrier.h"

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FLockJointBarrier : public FConstraintBarrier
{
public:
	FLockJointBarrier();
	FLockJointBarrier(FLockJointBarrier&& Other) = default;
	FLockJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FLockJointBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier* Rb1, const FVector* FramePosition1, const FQuat* FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector* FramePosition2, const FQuat* FrameRotation2) override;

private:
	FLockJointBarrier(const FLockJointBarrier&) = delete;
	void operator=(const FLockJointBarrier&) = delete;
};
