// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/TrackWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "Vehicle/TrackWheelRef.h"

FTrackWheelBarrier::FTrackWheelBarrier()
	: NativeRef {new FTrackWheelRef}
{
}

FTrackWheelBarrier::FTrackWheelBarrier(std::unique_ptr<FTrackWheelRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FTrackWheelBarrier::FTrackWheelBarrier(FTrackWheelBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FTrackWheelRef);
}

FTrackWheelBarrier::~FTrackWheelBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrackWheelRef.
}

bool FTrackWheelBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

FRigidBodyBarrier FTrackWheelBarrier::GetRigidBody() const
{
	check(HasNative());

	return FRigidBodyBarrier(std::make_unique<FRigidBodyRef>(NativeRef->Native->getRigidBody()));
}