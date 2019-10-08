// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Constraints/Constraint1DOFBarrier.h"

class FRigidBodyBarrier;

class AGXUNREALBARRIER_API FPrismaticBarrier : public FConstraint1DOFBarrier
{
public:
	FPrismaticBarrier();
	virtual ~FPrismaticBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier *Rb1, const FVector *FramePosition1, const FQuat *FrameRotation1,
		const FRigidBodyBarrier *Rb2, const FVector *FramePosition2, const FQuat *FrameRotation2,
		const UWorld *World) override;

private:
	FPrismaticBarrier(const FPrismaticBarrier&) = delete;
	void operator=(const FPrismaticBarrier&) = delete;
};
