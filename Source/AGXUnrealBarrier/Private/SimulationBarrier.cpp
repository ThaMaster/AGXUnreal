#include "SimulationBarrier.h"

#include "RigidBodyBarrier.h"
#include "TerrainBarrier.h"
#include "TypeConversions.h"
#include "Constraints/ConstraintBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/MaterialBarrier.h"
#include "AGX_LogCategory.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxSDK/Simulation.h>
#include "EndAGXIncludes.h"

#include "Misc/AssertionMacros.h"

FSimulationBarrier::FSimulationBarrier()
	: NativeRef {new FSimulationRef}
{
}

FSimulationBarrier::~FSimulationBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FSimulationRef.
}

void FSimulationBarrier::AddRigidBody(FRigidBodyBarrier* Body)
{
	check(HasNative());
	check(Body->HasNative());
	NativeRef->Native->add(Body->GetNative()->Native);
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

void FSimulationBarrier::AddContactMaterial(FContactMaterialBarrier* ContactMaterial)
{
	check(HasNative());
	check(ContactMaterial->HasNative());
	NativeRef->Native->add(ContactMaterial->GetNative()->Native);
}

void FSimulationBarrier::RemoveContactMaterial(FContactMaterialBarrier* ContactMaterial)
{
	check(HasNative());
	check(ContactMaterial->HasNative());
	NativeRef->Native->remove(ContactMaterial->GetNative()->Native);
}

void FSimulationBarrier::AddTerrain(FTerrainBarrier* Terrain)
{
	check(HasNative());
	check(Terrain->HasNative());
	NativeRef->Native->add(Terrain->GetNative()->Native);
}

bool FSimulationBarrier::WriteAGXArchive(const FString& Filename) const
{
	check(HasNative());
	size_t NumObjectsWritten = NativeRef->Native->write(Convert(Filename));
	if (NumObjectsWritten == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("Native simulation reported zero written objects."));
		return false;
	}

	return true; /// \todo How do we determine if all objects were successfully written?
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
