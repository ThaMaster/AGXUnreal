// Copyright 2024, Algoryx Simulation AB.

#include "Wire/WireParameterControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/Wire/WireParameterControllerPtr.h"

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
