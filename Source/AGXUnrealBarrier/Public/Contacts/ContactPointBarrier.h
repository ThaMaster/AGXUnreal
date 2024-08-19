// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

// Standard library includes.
#include <memory>

struct FContactPointEntity;

class AGXUNREALBARRIER_API FContactPointBarrier
{
public:
	FContactPointBarrier();
	FContactPointBarrier(std::unique_ptr<FContactPointEntity> InNativeEntity);
	FContactPointBarrier(FContactPointBarrier&& InOther);
	FContactPointBarrier(const FContactPointBarrier& InOther);

	~FContactPointBarrier();

	FContactPointBarrier& operator=(const FContactPointBarrier& InOther);

	// Collision detection state getters.
	bool IsEnabled() const;
	FVector GetLocation() const;
	FVector GetNormal() const;
	FVector GetTangentU() const;
	FVector GetTangentV() const;
	double GetDepth() const;
	FVector GetVelocity() const;
	FVector GetWitnessPoint(int32 Index) const;
	float GetArea() const;

	// Collision detection state setters. May only be called before the solver.
	void SetEnable(bool Enable);
	void SetLocation(const FVector& Location);
	void SetNormal(const FVector& Normal);
	void SetTangentU(const FVector& TangentU);
	void SetTangentV(const FVector& TangentV);
	void SetDepth(const double Depth);
	void SetVelocity(const FVector& Velocity);

	// Solver state getters. May only be called after the solver.
	FVector GetForce() const;
	double GetForceMagnitude() const;
	FVector GetNormalForce() const;
	double GetNormalForceMagnitude() const;
	FVector GetTangentialForce() const;
	double GetTangentialForceUMagnitude() const;
	double GetTangentialForceVMagnitude() const;
	double GetTangentialForceMagnitude() const;
	FVector GetLocalForce() const;
	double GetLocalForce(int32 Index);

	// Native management.
	bool HasNative() const;
	FContactPointEntity* GetNative();
	const FContactPointEntity* GetNative() const;

private:
	std::unique_ptr<FContactPointEntity> NativeEntity;
};
