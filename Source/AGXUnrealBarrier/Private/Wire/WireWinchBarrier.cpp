#include "Wire/WireWinchBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "TypeConversions.h"
#include "Wire/WireWinchRef.h"
#include "NativeBarrier.impl.h"

FWireWinchBarrier::FWireWinchBarrier()
	: Super()
{
}

FWireWinchBarrier::FWireWinchBarrier(std::unique_ptr<FWireWinchRef> Native)
	: Super(std::move(Native))
{
}

FWireWinchBarrier::FWireWinchBarrier(FWireWinchBarrier&& Other)
	: Super(std::move(Other))
{
}

FWireWinchBarrier::~FWireWinchBarrier()
{
}

/// The body that the winch is attached to. Will be empty when attached to the world.
FRigidBodyBarrier FWireWinchBarrier::GetRigidBody() const
{
	check(HasNative());
	return AGXBarrierFactories::CreateRigidBodyBarrier(NativeRef->Native->getRigidBody());
}

// See commend on member function declaration. Remove this if the declaration has been removed.
#if 0
/// The position of the winch on the body it's attached to, or in world space if there is no
/// body.
FVector FWireWinchBarrier::GetLocalPosition() const
{
	check(HasNative());
	NativeRef->Native->get???()
}
#endif

/// The direction of the winch on the body it's attached to, or in world space if there is no
/// body.
FVector FWireWinchBarrier::GetLocalNormal() const
{
	check(HasNative());
	return ConvertVector(NativeRef->Native->getNormal());
}

/// Maximum force to push or pull the wire.
FAGX_DoubleInterval FWireWinchBarrier::GetForceRange() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getForceRange());
}

/// The ability of the winch to slow down the wire when the brake is enabled.
FAGX_DoubleInterval FWireWinchBarrier::GetBrakeForceRange() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getBrakeForceRange());
}

/// Whether or not the winch is currently braking.
bool FWireWinchBarrier::IsBrakeEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableForcedBrake();
}

/// The speed that wire is being pulled in or payed out with.
double FWireWinchBarrier::GetSpeed() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSpeed());
}

/// Whether route nodes should take wire from the winch,
//		or create new wire.
bool FWireWinchBarrier::IsAutoFeed() const
{
	check(HasNative());
	return NativeRef->Native->getAutoFeed();
}

// See commend on member function declaration in the header file. Remove this if the declaration
// has been removed.
#if 0
/// The length of wire that the winch contains.
double FWireWinchBarrier::GetWireLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->get???());
}
#endif

/// The length of wire that the winch contains currently.
double FWireWinchBarrier::GetPulledInLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getPulledInWireLength());
}

FGuid FWireWinchBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

void FWireWinchBarrier::AllocateNative(
	const FRigidBodyBarrier* Body, const FVector& PositionInBody, const FVector& NormalInBody,
	double PulledInLength)
{
	PreNativeChanged();
	NativeRef->Native = new agxWire::WireWinchController(
		Body->GetNative()->Native, ConvertDisplacement(PositionInBody), ConvertVector(NormalInBody),
		PulledInLength);
	PostNativeChanged();
}

// See comment on declarations in header file. Remove these if the declarations has been removed.
#if 0
void FWireWinchBarrier::PreNativeChanged()
{
}

void FWireWinchBarrier::PostNativeChanged()
{
}
#endif
