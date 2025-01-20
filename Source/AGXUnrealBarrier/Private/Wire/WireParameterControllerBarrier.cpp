// Copyright 2024, Algoryx Simulation AB.

#include "Wire/WireParameterControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Wire/WireParameterControllerPtr.h"
#include "TypeConversions.h"

FWireParameterControllerBarrier::FWireParameterControllerBarrier()
	: NativePtr {new FWireParameterControllerPtr()}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	std::unique_ptr<FWireParameterControllerPtr> Native)
	: NativePtr {std::move(Native)}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	const FWireParameterControllerBarrier& Other)
	: NativePtr {new FWireParameterControllerPtr(Other.NativePtr->NativeWire)}
{
}

FWireParameterControllerBarrier::FWireParameterControllerBarrier(
	FWireParameterControllerBarrier&& Other)
	: NativePtr {std::move(Other.NativePtr)}
{
}

FWireParameterControllerBarrier::~FWireParameterControllerBarrier()
{
	// Destructor definition must be in the .cpp file since the definition, not just the
	// declaration, of FWireParameterControllerPtr must be available.
}

FWireParameterControllerBarrier& FWireParameterControllerBarrier::operator=(
	const FWireParameterControllerBarrier& Other)
{
	*NativePtr = *Other.NativePtr;
	return *this;
}

void FWireParameterControllerBarrier::SetMaximumContactMovementOneTimestep(double MaxMovement)
{
	check(HasNative());
	const double MaxMovementAGX = ConvertDistanceToAGX(MaxMovement);
	NativePtr->NativeWire->getParameterController()->setMaximumContactMovementOneTimestep(
		MaxMovementAGX);
}

double FWireParameterControllerBarrier::GetMaximumContactMovementOneTimestep() const
{
	check(HasNative());
	const double MaxMovementAGX =
		NativePtr->NativeWire->getParameterController()->getMaximumContactMovementOneTimestep();
	const double MaxMovement = ConvertDistanceToUnreal<double>(MaxMovementAGX);
	return MaxMovement;
}

void FWireParameterControllerBarrier::SetMinimumDistanceBetweenNodes(double MinDistance)
{
	check(HasNative());
	const double MinDistanceAGX = ConvertDistanceToAGX(MinDistance);
	NativePtr->NativeWire->getParameterController()->setMinimumDistanceBetweenNodes(MinDistanceAGX);
}

double FWireParameterControllerBarrier::GetMinimumDistanceBetweenNodes() const
{
	check(HasNative());
	const double MinDistanceAGX =
		NativePtr->NativeWire->getParameterController()->getMinimumDistanceBetweenNodes();
	const double MinDistance = ConvertDistanceToUnreal<double>(MinDistanceAGX);
	return MinDistance;
}

void FWireParameterControllerBarrier::SetScaleConstant(double ScaleConstant)
{
	check(HasNative());
	NativePtr->NativeWire->getParameterController()->setScaleConstant(ScaleConstant);
}

double FWireParameterControllerBarrier::GetScaleConstant() const
{
	check(HasNative());
	return NativePtr->NativeWire->getParameterController()->getScaleConstant();
}

bool FWireParameterControllerBarrier::HasNative() const
{
	return NativePtr->NativeWire != nullptr;
}
