#include "RigidBodyBarrier.h"

#include "AGXRefs.h"


// TODO: Move these conversion helpers somewhere the entire Barrier module can
//       access them.
namespace
{
	float convert(agx::Real v)
	{
		return static_cast<float>(v);
	}

	agx::Real convert(float v)
	{
		return static_cast<agx::Real>(v);
	}

	FVector convert(agx::Vec3 v)
	{
		return FVector(convert(v.x()), convert(v.y()), convert(v.z()));
	}

	agx::Vec3 convert(FVector v)
	{
		return agx::Vec3(convert(v.X), convert(v.Y), convert(v.Z));
	}
}

FRigidBodyBarrier::FRigidBodyBarrier()
	: NativeRef{new FRigidBodyRef}
{
	static bool IsAGXInitialized = false;
	if (!IsAGXInitialized)
	{
		agx::init();
		IsAGXInitialized = true;
	}

	// TODO: This is test code and should be removed.
	NativeRef->Native = new agx::RigidBody();
	std::cout << NativeRef->Native->getVelocity() << '\n';
}

FRigidBodyBarrier::~FRigidBodyBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniue_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FRigidBodyRef.
}

void FRigidBodyBarrier::SetPosition(FVector PositionUnreal)
{
	if (NativeRef->Native == nullptr)
	{
		return;
	}

	agx::Vec3 PositionAGX = convert(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FRigidBodyBarrier::GetPosition() const
{
	if (NativeRef->Native == nullptr)
	{
		return FVector();
	}

	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	FVector PositionUnreal = convert(PositionAGX);
	return PositionUnreal;
}

void FRigidBodyBarrier::SetMass(float MassUnreal)
{
	if (NativeRef->Native != nullptr)
	{
		return;
	}
	agx::Real MassAGX = convert(MassUnreal);
	NativeRef->Native->getMassProperties()->setMass(MassAGX);
}


float FRigidBodyBarrier::GetMass()
{
	if (NativeRef->Native != nullptr)
	{
		return float();
	}
	agx::Real MassAGX = NativeRef->Native->getMassProperties()->getMass();
	float MassUnreal = convert(MassAGX);
	return MassUnreal;
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