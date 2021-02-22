#include "SimulationBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Shapes/Sensors/SensorContactData.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TireBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/Simulation.h>
#include <agx/Statistics.h>
#include <agx/UniformGravityField.h>
#include <agx/PointGravityField.h>
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

void FSimulationBarrier::AddTire(FTireBarrier* Tire)
{
	check(HasNative());
	check(Tire->HasNative());
	NativeRef->Native->add(Tire->GetNative()->Native);
}

void FSimulationBarrier::SetEnableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool CanCollide)
{
	check(HasNative());

	// In AGXUnreal, adding a collision group pair always mean "disable collision between these
	// groups". Therefore, the collision enable flag is always set to false.
	// Note that internally, the collision group names are converted to a 32 bit unsigned int via a
	// hash function.
	NativeRef->Native->getSpace()->setEnablePair(
		StringTo32BitFnvHash(Group1.ToString()), StringTo32BitFnvHash(Group2.ToString()),
		CanCollide);
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

void FSimulationBarrier::SetTimeStep(float TimeStep)
{
	check(HasNative());
	NativeRef->Native->setTimeStep(Convert(TimeStep));
}

float FSimulationBarrier::GetTimeStep() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getTimeStep());
}

void FSimulationBarrier::SetNumPpgsIterations(int32 NumIterations)
{
	check(HasNative());
	check(NumIterations > 0);
	agx::Int NumIterationsAgx = Convert(NumIterations);
	NativeRef->Native->getSolver()->setNumPPGSRestingIterations(NumIterationsAgx);
}

int32 FSimulationBarrier::GetNumPpgsIterations() const
{
	check(HasNative());
	agx::UInt NumIterationsAgx = NativeRef->Native->getSolver()->getNumPPGSRestingIterations();
	if (NumIterationsAgx > std::numeric_limits<int32>::max())
	{
		NumIterationsAgx = std::numeric_limits<int32>::max();
		UE_LOG(
			LogAGX, Warning, TEXT("Too many PPGS resting iterations. Value clamped to %llu."),
			NumIterationsAgx);
	}
	int32 NumIterations = static_cast<int32>(NumIterationsAgx);
	return NumIterations;
}

void FSimulationBarrier::SetUniformGravity(const FVector& Gravity)
{
	check(HasNative());

	agx::Vec3 GravityAgx = ConvertVector(Gravity);
	agx::UniformGravityFieldRef Field = new agx::UniformGravityField(GravityAgx);
	NativeRef->Native->setGravityField(Field);
}

FVector FSimulationBarrier::GetUniformGravity() const
{
	agx::GravityField* GravityField = NativeRef->Native->getGravityField();
	if (!GravityField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetUniformGravity failed, native Simulation does not have a gravity field."));
		return FVector();
	}

	agx::UniformGravityField* UniformField = dynamic_cast<agx::UniformGravityField*>(GravityField);
	if (!UniformField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetUniformGravity called on Simulation with a Gravity Field that is not a "
				 "UniformGravityField."));
		return FVector();
	}

	return ConvertVector(UniformField->getGravity());
}

void FSimulationBarrier::SetPointGravity(const FVector& Origin, float Magnitude)
{
	// Magnitude from cm/s^2 to m/s^2.
	agx::Real MagnitudeAgx = ConvertDistance(Magnitude);
	agx::Vec3 OriginAgx = ConvertVector(Origin);

	agx::PointGravityFieldRef Field = new agx::PointGravityField(OriginAgx, MagnitudeAgx);
	NativeRef->Native->setGravityField(Field);
}

FVector FSimulationBarrier::GetPointGravity(float& OutMagnitude) const
{
	agx::GravityField* GravityField = NativeRef->Native->getGravityField();
	if (!GravityField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointGravity failed, native Simulation does not have a gravity field."));
		OutMagnitude = 0.f;
		return FVector();
	}

	agx::PointGravityField* PointField = dynamic_cast<agx::PointGravityField*>(GravityField);
	if (!PointField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointGravity called on Simulation with a Gravity Field that is not a "
				 "PointGravityField."));
		OutMagnitude = 0.f;
		return FVector();
	}

	OutMagnitude = Convert(PointField->getGravity());
	return ConvertVector(PointField->getCenter());
}

namespace
{
	template <typename T>
	FGuid GetGuid(const T* NativeObject)
	{
		if (!NativeObject)
		{
			FGuid Guid;
			Guid.Invalidate();
			return Guid;
		}

		return Convert(NativeObject->getUuid());
	}
}

TArray<FSensorContactData> FSimulationBarrier::GetSensorContactData(
	const FShapeBarrier& Shape) const
{
	check(HasNative());
	check(Shape.HasNative());

	TArray<FSensorContactData> SensorContactDataArr;
	agxCollide::GeometryContactPtrVector ContactsAgx;
	NativeRef->Native->getSpace()->getGeometryContacts(
		ContactsAgx, Shape.GetNative()->NativeGeometry);

	SensorContactDataArr.Reserve(ContactsAgx.size());
	for (const agxCollide::GeometryContact* Gc : ContactsAgx)
	{
		const agxCollide::ContactPointVector Points = Gc->points();
		FSensorContactData ContactData;

		ContactData.FirstShapeGuid = GetGuid<agxCollide::Geometry>(Gc->geometry(0));
		ContactData.SecondShapeGuid = GetGuid<agxCollide::Geometry>(Gc->geometry(1));
		ContactData.FirstBodyGuid = GetGuid<agx::RigidBody>(Gc->rigidBody(0));
		ContactData.SecondBodyGuid = GetGuid<agx::RigidBody>(Gc->rigidBody(1));

		// Set contact points data.
		ContactData.Points.Reserve(Points.size());
		for (const agxCollide::ContactPoint& Point : Points)
		{
			FSensorContactPoint PointData;
			PointData.Position = ConvertVector(Point.point());
			PointData.Force = ConvertVector(Point.getForce());
			PointData.NomalForce = ConvertVector(Point.getNormalForce());
			PointData.Normal = ConvertFloatVector(Point.normal());
			PointData.Depth = Convert(Point.depth());
			PointData.Area = Convert(Point.area());
			ContactData.Points.Add(PointData);
		}

		SensorContactDataArr.Add(ContactData);
	}
	return SensorContactDataArr;
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

namespace
{
	float GetStatisticsTime(void* Instance, const char* Name)
	{
		agx::Statistics::Data<agx::Real>* Entry =
			agx::Statistics::instance()->getData<agx::Real>(Instance, Name);
		if (Entry == nullptr)
		{
			/// @todo Unclear where this message should be. Not here, since the user may not be
			/// interested in this particular part of the statistics.
			// UE_LOG(
			//	LogAGX, Warning, TEXT("Could not get '%s' from AGX Dynamics Statistics."),
			//	UTF8_TO_TCHAR(Name));
			return -1.0f;
		}
		return Convert(Entry->value());
	}

	int32 GetStatisticsCount(void* Instance, const char* Name)
	{
		agx::Statistics::Data<size_t>* Entry =
			agx::Statistics::instance()->getData<size_t>(Instance, Name);
		if (Entry == nullptr)
		{
			return -1;
		}
		size_t ValueAgx = Entry->value();
		const size_t MaxAllowed = std::numeric_limits<int32>::max();
		if (ValueAgx > MaxAllowed)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Statistics value %llu for '%s' is too large, truncated to %d."),
				(unsigned long long) ValueAgx, UTF8_TO_TCHAR(Name), MaxAllowed);
			ValueAgx = MaxAllowed;
		}
		return static_cast<int32>(ValueAgx);
	}
};

FAGX_Statistics FSimulationBarrier::GetStatistics()
{
	FAGX_Statistics Statistics;

	void* SimulationContext = static_cast<void*>(NativeRef->Native.get());
	Statistics.StepForwardTime = GetStatisticsTime(SimulationContext, "Step forward time");
	Statistics.PreCollideTime = GetStatisticsTime(SimulationContext, "Pre-collide event time");
	Statistics.ContactEventsTime =
		GetStatisticsTime(SimulationContext, "Triggering contact events");
	Statistics.PreStepTime = GetStatisticsTime(SimulationContext, "Pre-step event time");
	Statistics.DynamicsSystemTime = GetStatisticsTime(SimulationContext, "Dynamics-system time");
	Statistics.SpaceTime = GetStatisticsTime(SimulationContext, "Collision-detection time");
	Statistics.PostStepTime = GetStatisticsTime(SimulationContext, "Post-step event time");
	Statistics.LastStepTime = GetStatisticsTime(SimulationContext, "Last-step event time");
	Statistics.InterStepTime = GetStatisticsTime(SimulationContext, "Inter-step time");
	Statistics.NumParticles = GetStatisticsCount(SimulationContext, "Num particles");

	void* DynamicsContext = static_cast<void*>(NativeRef->Native.get()->getDynamicsSystem());
	Statistics.NumBodies = GetStatisticsCount(DynamicsContext, "Num enabled rigid bodies");
	Statistics.NumConstraints = GetStatisticsCount(DynamicsContext, "Num binary constraints") +
								GetStatisticsCount(DynamicsContext, "Num multi-body constraints");
	Statistics.NumContacts = GetStatisticsCount(DynamicsContext, "Num contact constraints");

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
