#include "Wire/WireWinchBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "TypeConversions.h"
#include "Wire/WireWinchRef.h"
#include "NativeBarrier.impl.h"

template class AGXUNREALBARRIER_API FNativeBarrier<FWireWinchRef>;

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

void FWireWinchBarrier::SetPulledInLength(double InPulledInLength)
{
	check(HasNative());
	NativeRef->Native->setPulledInWireLength(InPulledInLength);
}

double FWireWinchBarrier::GetPulledInLength() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getPulledInWireLength());
}

void FWireWinchBarrier::SetAutoFeed(bool bAutoFeed)
{
	check(HasNative());
	NativeRef->Native->setAutoFeed(bAutoFeed);
}

bool FWireWinchBarrier::GetAutoFeed() const
{
	check(HasNative());
	return NativeRef->Native->getAutoFeed();
}

void FWireWinchBarrier::SetForceRange(const FAGX_DoubleInterval& InForceRange)
{
	check(HasNative());
	NativeRef->Native->setForceRange(Convert(InForceRange));
}

/// Maximum force to push or pull the wire.
FAGX_DoubleInterval FWireWinchBarrier::GetForceRange() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getForceRange());
}

void FWireWinchBarrier::SetBrakeForceRange(const FAGX_DoubleInterval& InBrakeForceRange)
{
	check(HasNative());
	NativeRef->Native->setBrakeForceRange(Convert(InBrakeForceRange));
}

/// The ability of the winch to slow down the wire when the brake is enabled.
FAGX_DoubleInterval FWireWinchBarrier::GetBrakeForceRange() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getBrakeForceRange());
}

void FWireWinchBarrier::SetBrakeEnabled(bool bBrakeEnabled)
{
	check(HasNative());
	NativeRef->Native->setEnableForcedBrake(bBrakeEnabled);
}

/// Whether or not the winch is currently braking.
bool FWireWinchBarrier::IsBrakeEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnableForcedBrake();
}

void FWireWinchBarrier::SetTargetSpeed(double InTargetSpeed)
{
	check(HasNative());
	agx::Real TargetSpeedAGX = ConvertDistanceToAgx(InTargetSpeed);
	NativeRef->Native->setSpeed(TargetSpeedAGX);
}

/// The speed that wire is being pulled in or payed out with.
double FWireWinchBarrier::GetTargetSpeed() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSpeed());
}

double FWireWinchBarrier::GetCurrentSpeed() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getCurrentSpeed());
}

/// Whether route nodes should take wire from the winch,
//		or create new wire.
bool FWireWinchBarrier::IsAutoFeed() const
{
	check(HasNative());
	return NativeRef->Native->getAutoFeed();
}

FGuid FWireWinchBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

void FWireWinchBarrier::AllocateNative(
	const FRigidBodyBarrier* Body, const FVector& LocalLocation, const FVector& LocalNormal,
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
