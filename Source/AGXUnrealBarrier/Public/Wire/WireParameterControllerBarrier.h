// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FWireParameterControllerPtr;

class AGXUNREALBARRIER_API FWireParameterControllerBarrier
{
public:
	FWireParameterControllerBarrier();
	explicit FWireParameterControllerBarrier(std::unique_ptr<FWireParameterControllerPtr> Native);
	FWireParameterControllerBarrier(const FWireParameterControllerBarrier& Other);
	FWireParameterControllerBarrier(FWireParameterControllerBarrier&& Other);
	~FWireParameterControllerBarrier();
	FWireParameterControllerBarrier& operator=(const FWireParameterControllerBarrier& Other);

	void SetMaximumContactMovementOneTimestep(double MaxMovement);
	double GetMaximumContactMovementOneTimestep() const;

	void SetMinimumDistanceBetweenNodes(double MinDistance);
	double GetMinimumDistanceBetweenNodes() const;

	void SetRadiusMultiplier(double RadiusMultiplier);
	double GetRadiusMultiplier() const;
	double GetScaledRadiusMultiplier(double WireRadius) const;

	void SetScaleConstant(double ScaleConstant);
	double GetScaleConstant() const;

	bool HasNative() const;

private:
	std::unique_ptr<FWireParameterControllerPtr> NativePtr;
};
