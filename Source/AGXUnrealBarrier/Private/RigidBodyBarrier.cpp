#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

#include <agx/Vec3.h>
#include <agx/Quat.h>

FRigidBodyBarrier::FRigidBodyBarrier()
	: NativeRef {new FRigidBodyRef}
{
}

FRigidBodyBarrier::FRigidBodyBarrier(std::unique_ptr<FRigidBodyRef> Native)
	: NativeRef(std::move(Native))
{
}

FRigidBodyBarrier::FRigidBodyBarrier(FRigidBodyBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRigidBodyRef);
}

FRigidBodyBarrier::~FRigidBodyBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FRigidBodyRef.
}

void FRigidBodyBarrier::SetPosition(FVector PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertVector(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FRigidBodyBarrier::GetPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	FVector PositionUnreal = ConvertVector(PositionAGX);
	return PositionUnreal;
}

void FRigidBodyBarrier::SetRotation(FQuat RotationUnreal)
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

void FRigidBodyBarrier::SetVelocity(FVector VelocityUnreal)
{
	check(HasNative());
	agx::Vec3 VelocityAGX = ConvertVector(VelocityUnreal);
	NativeRef->Native->setVelocity(VelocityAGX);
}

FVector FRigidBodyBarrier::GetVelocity() const
{
	check(HasNative());
	agx::Vec3 VelocityAGX = NativeRef->Native->getVelocity();
	FVector VelocityUnreal = ConvertVector(VelocityAGX);
	return VelocityUnreal;
}

void FRigidBodyBarrier::SetAngularVelocity(FVector AngularVelocityUnreal)
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

void FRigidBodyBarrier::SetMass(float MassUnreal)
{
	check(HasNative());
	agx::Real MassAGX = Convert(MassUnreal);
	NativeRef->Native->getMassProperties()->setMass(MassAGX);
}

float FRigidBodyBarrier::GetMass() const
{
	check(HasNative());
	agx::Real MassAGX = NativeRef->Native->getMassProperties()->getMass();
	float MassUnreal = Convert(MassAGX);
	return MassUnreal;
}

void FRigidBodyBarrier::SetInertiaTensorDiagonal(const FVector& InertiaUnreal)
{
	check(HasNative())
	agx::Vec3 InertiaAGX = Convert(InertiaUnreal);
	NativeRef->Native->getMassProperties()->setInertiaTensor(InertiaAGX);
}

FVector FRigidBodyBarrier::GetInertiaTensorDiagonal() const
{
	check(HasNative())
	agx::Vec3 InertiaAGX = NativeRef->Native->getMassProperties()->getInertiaTensor().getDiagonal();
	FVector InertiaUnreal = Convert(InertiaAGX);
	return InertiaUnreal;
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

bool FRigidBodyBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FRigidBodyBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agx::RigidBody();
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

void FRigidBodyBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
