#include "SimulationBarrier.h"

#include "RigidBodyBarrier.h"
#include "Constraints/ConstraintBarrier.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxSDK/Simulation.h>
#include "EndAGXIncludes.h"

#include "Misc/AssertionMacros.h"

FSimulationBarrier::FSimulationBarrier()
	: NativeRef{new FSimulationRef}
{
}

FSimulationBarrier::~FSimulationBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniue_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FSimulationRef.
}

void FSimulationBarrier::AddRigidBody(FRigidBodyBarrier* body)
{
	check(HasNative());
	check(body->HasNative());
	NativeRef->Native->add(body->GetNative()->Native);
}

void FSimulationBarrier::AddConstraint(FConstraintBarrier* Constraint)
{
	check(HasNative());
	check(Constraint->HasNative());
	NativeRef->Native->add(Constraint->GetNative()->Native);
}

void FSimulationBarrier::AddMaterial(FMaterialBarrier* Material)
{
	check(HasNative());
	check(Material->HasNative());
	NativeRef->Native->add(Material->GetNative()->Native);
}

void FSimulationBarrier::RemoveMaterial(FMaterialBarrier* Material)
{
	check(HasNative());
	check(Material->HasNative());
	NativeRef->Native->remove(Material->GetNative()->Native);
}

void FSimulationBarrier::Step()
{
	check(HasNative());
	NativeRef->Native->stepForward();
}

bool FSimulationBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FSimulationBarrier::AllocateNative()
{
	NativeRef->Native = new agxSDK::Simulation();
}

FSimulationRef* FSimulationBarrier::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

const FSimulationRef* FSimulationBarrier::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

void FSimulationBarrier::ReleaseNative()
{
	NativeRef = nullptr;
}
