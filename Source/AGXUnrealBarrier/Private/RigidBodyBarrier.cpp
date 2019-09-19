#include "RigidBodyBarrier.h"

#include "AGXRefs.h"
#include "TypeConversions.h"

// TODO: Remove.
#include "BeginAGXIncludes.h"
#include <agxSDK/Simulation.h>
#include "EndAGXIncludes.h"

FRigidBodyBarrier::FRigidBodyBarrier()
	: NativeRef{new FRigidBodyRef}
{
	// TODO: This is debug code. Remove.
	static bool IsAGXInitialized = false;
	if (!IsAGXInitialized)
	{
		agx::init();
		IsAGXInitialized = true;
	}
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

	agx::Vec3 PositionAGX = Convert(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FRigidBodyBarrier::GetPosition() const
{
	if (NativeRef->Native == nullptr)
	{
		return FVector();
	}

	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	FVector PositionUnreal = Convert(PositionAGX);
	return PositionUnreal;
}

void FRigidBodyBarrier::SetMass(float MassUnreal)
{
	if (NativeRef->Native != nullptr)
	{
		return;
	}
	agx::Real MassAGX = Convert(MassUnreal);
	NativeRef->Native->getMassProperties()->setMass(MassAGX);
}

float FRigidBodyBarrier::GetMass()
{
	if (NativeRef->Native != nullptr)
	{
		return float();
	}
	agx::Real MassAGX = NativeRef->Native->getMassProperties()->getMass();
	float MassUnreal = Convert(MassAGX);
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

FRigidBodyRef* FRigidBodyBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

// TODO: This is test code and should be removed.
void FRigidBodyBarrier::DebugSimulate()
{
	agxSDK::SimulationRef simulation = new agxSDK::Simulation();
	simulation->add(NativeRef->Native);
	for (int I = 0; I < 10; ++I)
	{
		simulation->stepForward();
		std::cout << "At time step " << I << ": velocity is " << NativeRef->Native->getVelocity() << " m/s\n";
	}
}
