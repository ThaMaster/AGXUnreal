// Copyright 2023, Algoryx Simulation AB.

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

	bool IsEnabled() const;

	float GetDepth() const;
	FVector GetLocation() const;
	FVector GetNormal() const;
	FVector GetTangentU() const;
	FVector GetTangentV() const;

	FVector GetForce() const;
	FVector GetNormalForce() const;
	FVector GetTangentialForce() const;
	FVector GetLocalForce() const;

	FVector GetWitnessPoint(int32 Index) const;
	float GetArea() const;

	bool HasNative() const;
	FContactPointEntity* GetNative();
	const FContactPointEntity* GetNative() const;

private:
	std::unique_ptr<FContactPointEntity> NativeEntity;
};
