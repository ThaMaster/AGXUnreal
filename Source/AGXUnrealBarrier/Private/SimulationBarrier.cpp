#include "SimulationBarrier.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/Simulation.h>
#include <agx/Statistics.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
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

void FSimulationBarrier::AddShape(FShapeBarrier* Shape)
{
	check(HasNative());
	check(Shape->HasNative());
	NativeRef->Native->add(Shape->GetNative()->NativeGeometry);
}

void FSimulationBarrier::AddConstraint(FConstraintBarrier* Constraint)
{
	check(HasNative());
	check(Constraint->HasNative());
	NativeRef->Native->add(Constraint->GetNative()->Native);
}

void FSimulationBarrier::AddShapeMaterial(FShapeMaterialBarrier* Material)
{
	check(HasNative());
	check(Material->HasNative());
	NativeRef->Native->add(Material->GetNative()->Native);
}

void FSimulationBarrier::RemoveShapeMaterial(FShapeMaterialBarrier* Material)
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

void FSimulationBarrier::SetDisableCollisionGroupPair(const FName& Group1, const FName& Group2)
{
	check(HasNative());

	// In AGXUnreal, adding a collision group pair always mean "disable collision between these
	// groups". Therefore, the collision enable flag is always set to false.
	// Note that internally, the collision group names are converted to a 32 bit unsigned int via a
	// hash function.
	NativeRef->Native->getSpace()->setEnablePair(
		StringTo32BitFnvHash(Group1.ToString()), StringTo32BitFnvHash(Group2.ToString()), false);
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

void FSimulationBarrier::EnableRemoteDebugging(int16 Port)
{
	check(HasNative());
	NativeRef->Native->setEnableRemoteDebugging(true, Port);
}

void FSimulationBarrier::Step()
{
	check(HasNative());
	NativeRef->Native->stepForward();
}

float FSimulationBarrier::GetTimeStamp() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getTimeStamp());
}

void FSimulationBarrier::SetTimeStamp(float TimeStamp)
{
	check(HasNative());
	NativeRef->Native->setTimeStamp(Convert(TimeStamp));
}

void FSimulationBarrier::SetStatisticsEnabled(bool bEnabled)
{
	agx::Statistics::instance()->setEnable(bEnabled);
}

FAGX_Statistics FSimulationBarrier::GetStatistics()
{
	FAGX_Statistics Statistics;

	agx::Statistics::Data<agx::Real>* StepForwardTime =
		agx::Statistics::instance()->getData<agx::Real>(
			NativeRef->Native.get(), "Step forward time");
	if (StepForwardTime == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Could not get step forward time from statistics."));
		Statistics.StepForwardTime = -1.0f;
	}
	else
	{
		Statistics.StepForwardTime = Convert(StepForwardTime->value());
	}

	agx::Statistics::Data<size_t>* NumParticles =
		agx::Statistics::instance()->getData<size_t>(NativeRef->Native.get(), "Num particles");
	if (NumParticles == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Could not get number of particles from statistics."));
		Statistics.NumParticles = -1;
	}
	else
	{
		Statistics.NumParticles = static_cast<uint32>(NumParticles->value());
	}

	return Statistics;
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
