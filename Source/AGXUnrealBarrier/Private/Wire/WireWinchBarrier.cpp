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

void FWireWinchBarrier::AllocateNative(
	const FRigidBodyBarrier* Body, const FVector& LocalLocation, const FVector& LocalNormal,
	double PulledInLength)
{
	PreNativeChanged();
	agx::RigidBody* NativeBody;
	if (Body != nullptr && Body->HasNative())
	{
		NativeBody = Body->GetNative()->Native;
	}
	else
	{
		NativeBody = nullptr;
	}

	NativeRef->Native = new agxWire::WireWinchController(
		NativeBody, ConvertDisplacement(LocalLocation), ConvertVector(LocalNormal),
		ConvertDistanceToAgx(PulledInLength));
	PostNativeChanged();
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
FVector FWireWinchBarrier::GetNormal() const
{
	check(HasNative());
	return ConvertVector(NativeRef->Native->getNormal());
}

FVector FWireWinchBarrier::GetLocation() const
{
	check(HasNative());
	const agx::Vec3 LocationAGX = NativeRef->Native->getStopNode()->getTranslate();
	const FVector Location = ConvertDisplacement(LocationAGX);
	return Location;
}

void FWireWinchBarrier::SetPulledInWireLength(double InPulledInLength)
{
	check(HasNative());
	agx::Real PulledInLengthAGX = ConvertDistanceToAgx(InPulledInLength);
	NativeRef->Native->setPulledInWireLength(PulledInLengthAGX);
}

double FWireWinchBarrier::GetPulledInWireLength() const
{
	check(HasNative());
	agx::Real PulledInLengthAGX = NativeRef->Native->getPulledInWireLength();
	return ConvertDistanceToUnreal<double>(PulledInLengthAGX);
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

void FWireWinchBarrier::SetSpeed(double InTargetSpeed)
{
	check(HasNative());
	agx::Real TargetSpeedAGX = ConvertDistanceToAgx(InTargetSpeed);
	NativeRef->Native->setSpeed(TargetSpeedAGX);
}

/// The speed that wire is being pulled in or payed out with.
double FWireWinchBarrier::GetSpeed() const
{
	check(HasNative());
	const agx::Real TargetSpeedAGX = NativeRef->Native->getSpeed();
	return ConvertDistanceToUnreal<double>(TargetSpeedAGX);
}

double FWireWinchBarrier::GetCurrentSpeed() const
{
	check(HasNative());
	agx::Real CurrentSpeedAGX = NativeRef->Native->getCurrentSpeed();
	return ConvertDistanceToUnreal<double>(CurrentSpeedAGX);
}

double FWireWinchBarrier::GetCurrentForce() const
{
	check(HasNative());
	return NativeRef->Native->getCurrentForce();
}

double FWireWinchBarrier::GetCurrentBrakeForce() const
{
	check(HasNative());
	return NativeRef->Native->getCurrentBrakeForce();
}

bool FWireWinchBarrier::HasWire() const
{
	check(HasNative());
	return NativeRef->Native->getConstraint() != nullptr;
}

/// @todo Figure out how to find the wire a winch is attached to. The below doesn't always work.
#if 0
FWireBarrier FWireWinchBarrier::GetWire() const
{
	check(HasNative());
	agxWire::WireWinchController* Winch = NativeRef->Native;
	agxWire::Node* Nodes[] {Winch->getStopNode(), Winch->getFixedNode()};
	for (agxWire::Node* Node : Nodes)
	{
		agx::RigidBody* Body = Node->getRigidBody();
		agxWire::Wire* Wire = agxWire::Wire::getWire(Body);
		if (Wire != nullptr)
		{
			return AGXBarrierFactories::CreateWireBarrier(Wire);
		}
	}
	return FWireBarrier();
}
#endif

FGuid FWireWinchBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
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
