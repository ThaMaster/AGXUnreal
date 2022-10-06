// Copyright 2022, Algoryx Simulation AB.

#include "Vehicle/TrackWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"
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

double FTrackWheelBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getRadius());
}

uint8 FTrackWheelBarrier::GetModel() const
{
	check(HasNative());
	return static_cast<uint8>(NativeRef->Native->getModel());
}

bool FTrackWheelBarrier::GetSplitSegments() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(agxVehicle::TrackWheel::SPLIT_SEGMENTS);
}

bool FTrackWheelBarrier::GetMoveNodesToRotationPlane() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(
		agxVehicle::TrackWheel::MOVE_NODES_TO_ROTATION_PLANE);
}

bool FTrackWheelBarrier::GetMoveNodesToWheel() const
{
	check(HasNative());
	return NativeRef->Native->getProperties().Is(agxVehicle::TrackWheel::MOVE_NODES_TO_WHEEL);
}

FVector FTrackWheelBarrier::GetRelativeLocation() const
{
	check(HasNative());
	agx::Frame* FrameAGX = NativeRef->Native->getLocalFrame();
	if (FrameAGX == nullptr)
	{
		return FVector::ZeroVector;
	}

	return ConvertDisplacement(FrameAGX->getLocalTranslate());
}

FRotator FTrackWheelBarrier::GetRelativeRotation() const
{
	check(HasNative());
	agx::Frame* FrameAGX = NativeRef->Native->getLocalFrame();
	if (FrameAGX == nullptr)
	{
		return FRotator::ZeroRotator;
	}

	return Convert(FrameAGX->getLocalRotate()).Rotator();
}