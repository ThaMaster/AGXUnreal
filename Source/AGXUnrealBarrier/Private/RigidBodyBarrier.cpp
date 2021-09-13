#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include <agx/Quat.h>
#include "EndAGXIncludes.h"

FRigidBodyBarrier::FRigidBodyBarrier()
	: NativeRef {new FRigidBodyRef}
{
}

FRigidBodyBarrier::FRigidBodyBarrier(std::unique_ptr<FRigidBodyRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
	MassProperties.BindTo(*NativeRef);
}

FRigidBodyBarrier::FRigidBodyBarrier(FRigidBodyBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRigidBodyRef);
	Other.MassProperties.BindTo(*Other.NativeRef);

	MassProperties.BindTo(*NativeRef);
}

FRigidBodyBarrier::~FRigidBodyBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FRigidBodyRef.
}

void FRigidBodyBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FRigidBodyBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FRigidBodyBarrier::SetPosition(const FVector& PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertDisplacement(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FRigidBodyBarrier::GetPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	FVector PositionUnreal = ConvertDisplacement(PositionAGX);
	return PositionUnreal;
}

void FRigidBodyBarrier::SetRotation(const FQuat& RotationUnreal)
{
	check(HasNative());
	agx::Quat RotationAGX = Convert(RotationUnreal);
	NativeRef->Native->setRotation(RotationAGX);
}

FQuat FRigidBodyBarrier::GetRotation() const
{
	check(HasNative());
	agx::Quat RotationAGX = NativeRef->Native->getRotation();
	FQuat RotationUnreal = Convert(RotationAGX);
	return RotationUnreal;
}

void FRigidBodyBarrier::SetVelocity(const FVector& VelocityUnreal)
{
	check(HasNative());
	agx::Vec3 VelocityAGX = ConvertDisplacement(VelocityUnreal);
	NativeRef->Native->setVelocity(VelocityAGX);
}

FVector FRigidBodyBarrier::GetVelocity() const
{
	check(HasNative());
	agx::Vec3 VelocityAGX = NativeRef->Native->getVelocity();
	FVector VelocityUnreal = ConvertDisplacement(VelocityAGX);
	return VelocityUnreal;
}

void FRigidBodyBarrier::SetAngularVelocity(const FVector& AngularVelocityUnreal)
{
	check(HasNative());
	agx::Vec3 AngularVelocityAGX = ConvertAngularVelocity(AngularVelocityUnreal);
	NativeRef->Native->setAngularVelocity(AngularVelocityAGX);
}

FVector FRigidBodyBarrier::GetAngularVelocity() const
{
	check(HasNative());
	agx::Vec3 AngularVelocityAGX = NativeRef->Native->getAngularVelocity();
	FVector AngularVelocityUnreal = ConvertAngularVelocity(AngularVelocityAGX);
	return AngularVelocityUnreal;
}

FMassPropertiesBarrier& FRigidBodyBarrier::GetMassProperties()
{
	return MassProperties;
}

const FMassPropertiesBarrier& FRigidBodyBarrier::GetMassProperties() const
{
	return MassProperties;
}

void FRigidBodyBarrier::UpdateMassProperties()
{
	check(HasNative());
	NativeRef->Native->updateMassProperties();
}

double FRigidBodyBarrier::CalculateMass() const
{
	check(HasNative());
	return NativeRef->Native->calculateMass();
}

void FRigidBodyBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FRigidBodyBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

FGuid FRigidBodyBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

void FRigidBodyBarrier::SetMotionControl(EAGX_MotionControl MotionControlUnreal)
{
	check(HasNative());
	agx::RigidBody::MotionControl MotionControlAGX = Convert(MotionControlUnreal);
	NativeRef->Native->setMotionControl(MotionControlAGX);
}

EAGX_MotionControl FRigidBodyBarrier::GetMotionControl() const
{
	check(HasNative());
	agx::RigidBody::MotionControl MotionControlAGX = NativeRef->Native->getMotionControl();
	EAGX_MotionControl MotionControlUnreal = Convert(MotionControlAGX);
	return MotionControlUnreal;
}

void FRigidBodyBarrier::AddShape(FShapeBarrier* Shape)
{
	check(HasNative());
	NativeRef->Native->add(Shape->GetNative()->NativeGeometry);
}

void FRigidBodyBarrier::AddForceAtCenterOfMass(const FVector& Force)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	NativeRef->Native->addForce(ForceAGX);
}

void FRigidBodyBarrier::AddForceAtLocalLocation(const FVector& Force, const FVector& Location)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	const agx::Vec3 LocationAGX = ConvertDisplacement(Location);
	NativeRef->Native->addForceAtLocalPosition(ForceAGX, LocationAGX);
}

void FRigidBodyBarrier::AddForceAtWorldLocation(const FVector& Force, const FVector& Location)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	const agx::Vec3 LocationAGX = ConvertDisplacement(Location);
	NativeRef->Native->addForceAtPosition(ForceAGX, LocationAGX);
}

FVector FRigidBodyBarrier::GetForce() const
{
	check(HasNative());
	const agx::Vec3 ForceAGX = NativeRef->Native->getForce();
	return ConvertVector(ForceAGX);
}

void FRigidBodyBarrier::AddTorqueWorld(const FVector& Torque)
{
	check(HasNative());
	/// \todo Is it correct to convert cm to m here?
	const agx::Vec3 TorqueAGX = ConvertDisplacement(Torque);
	NativeRef->Native->addTorque(TorqueAGX);
}

void FRigidBodyBarrier::AddTorqueLocal(const FVector& Torque)
{
	check(HasNative());
	/// \todo Is it correct to convert cm to m here?
	const agx::Vec3 TorqueAGX = ConvertDisplacement(Torque);
	NativeRef->Native->addLocalTorque(TorqueAGX);
}

FVector FRigidBodyBarrier::GetTorque() const
{
	check(HasNative());
	const agx::Vec3 TorqueAGX = NativeRef->Native->getTorque();
	/// \todo Is it correct to convert m to cm here?
	return ConvertDisplacement(TorqueAGX);
}

bool FRigidBodyBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FRigidBodyBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agx::RigidBody();
	MassProperties.BindTo(*NativeRef);
}

FRigidBodyRef* FRigidBodyBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRigidBodyRef* FRigidBodyBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FRigidBodyBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FRigidBodyBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		MassProperties.BindTo(*NativeRef);
		return;
	}

	NativeRef->Native = reinterpret_cast<agx::RigidBody*>(NativeAddress);
	MassProperties.BindTo(*NativeRef);
}

void FRigidBodyBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
