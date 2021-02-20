#pragma once

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_Statistics.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"

// System includes.
#include <memory>

struct FSimulationRef;
struct FSensorContactData;

class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeBarrier;
class FShapeMaterialBarrier;
class FTerrainBarrier;
class FTireBarrier;

class AGXUNREALBARRIER_API FSimulationBarrier
{
public:
	FSimulationBarrier();
	~FSimulationBarrier();

	void AddRigidBody(FRigidBodyBarrier* Body);
	void AddShape(FShapeBarrier* Shape);
	void AddConstraint(FConstraintBarrier* Constraint);

	void AddShapeMaterial(FShapeMaterialBarrier* Material);
	void RemoveShapeMaterial(FShapeMaterialBarrier* Material);

	void AddContactMaterial(FContactMaterialBarrier* ContactMaterial);
	void RemoveContactMaterial(FContactMaterialBarrier* ContactMaterial);

	void AddTerrain(FTerrainBarrier* Terrain);

	void AddTire(FTireBarrier* Tire);

	void SetEnableCollisionGroupPair(const FName& Group1, const FName& Group2, bool CanCollide);

	bool WriteAGXArchive(const FString& Filename) const;

	void EnableRemoteDebugging(int16 Port);

	void SetTimeStep(float TimeStep);
	float GetTimeStep() const;

	void SetNumPpgsIterations(int32 NumIterations);
	int32 GetNumPpgsIterations() const;

	void SetUniformGravity(const FVector& Gravity);
	FVector GetUniformGravity() const;

	void SetPointGravity(const FVector& Origin, float Magnitude);
	FVector GetPointGravity(float& OutMagnitude) const;

	// The sensor contact data is only valid during a single time step.
	TArray<FSensorContactData> GetSensorContactData(const FShapeBarrier& Shape) const;

	/**
	 * Perform one simulation step, moving the time stamp forward by one time step duration.
	 */
	void Step();

	/**
	 * The returned value is usually the amount of time that has been simulated, but SetTimeStamp
	 * may invalidate this assumption.
	 * @return The current simulation time stamp.
	 */
	float GetTimeStamp() const;

	/**
	 * Set the current simulation time stamp. Does not simulate to that time, just moves the clock
	 * hands.
	 * @param TimeStamp The new time stamp.
	 */
	void SetTimeStamp(float TimeStamp);

	/// \todo Statistics isn't a per-simulation thing in AGX Dynamics, so having statistics
	/// management here is a bit misleading.
	void SetStatisticsEnabled(bool bEnable);
	FAGX_Statistics GetStatistics();

	bool HasNative() const;
	void AllocateNative();
	FSimulationRef* GetNative();
	const FSimulationRef* GetNative() const;
	void ReleaseNative();

private:
	FSimulationBarrier(const FSimulationBarrier&) = delete;
	void operator=(const FSimulationBarrier&) = delete;

private:
	std::unique_ptr<FSimulationRef> NativeRef;
};
