// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ConstraintBarrier.h"

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FLockJointBarrier : public FConstraintBarrier
{
public:
	FLockJointBarrier();
	virtual ~FLockJointBarrier();

private:
	virtual void AllocateNativeImpl(const FRigidBodyBarrier *Rb1, const FRigidBodyBarrier *Rb2) override;

private:
	FLockJointBarrier(const FLockJointBarrier&) = delete;
	void operator=(const FLockJointBarrier&) = delete;
};
