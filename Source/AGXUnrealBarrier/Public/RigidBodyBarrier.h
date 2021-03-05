#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"
#include "MassPropertiesBarrier.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// System includes.
#include <memory>

struct FRigidBodyRef;

class FMassPropertiesBarrier;
class FShapeBarrier;

/**
 * Barrier between UAGX_RigidBody and agx::RigidBody. UAGX_RigidBody holds an
 * instance of RigidBodyBarrier and hidden behind the RigidBodyBarrier is a
 * agx::RigidBodyRef. This allows UAGX_RigidBody to interact with
 * agx::RigidBody without including agx/RigidBody.h.
 *
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
class AGXUNREALBARRIER_API FRigidBodyBarrier
{
public:
	FRigidBodyBarrier();
	FRigidBodyBarrier(std::unique_ptr<FRigidBodyRef> Native);
	FRigidBodyBarrier(FRigidBodyBarrier&& Other);
	~FRigidBodyBarrier();

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetPosition(FVector Position);
	FVector GetPosition() const;

	void SetRotation(FQuat Rotation);
	FQuat GetRotation() const;

	void SetVelocity(FVector Velocity);
	FVector GetVelocity() const;

	// In degrees/s.
	void SetAngularVelocity(FVector AngularVelocity);

	// In degrees/s.
	FVector GetAngularVelocity() const;

	FMassPropertiesBarrier& GetMassProperties();
	const FMassPropertiesBarrier& GetMassProperties() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	void SetMotionControl(EAGX_MotionControl MotionControl);
	EAGX_MotionControl GetMotionControl() const;

	FGuid GetGuid() const;

	void AddShape(FShapeBarrier* Shape);

	void AddForceAtCenterOfMass(const FVector& Force);
	void AddForceAtLocalLocation(const FVector& Force, const FVector& Location);
	void AddForceAtWorldLocation(const FVector& Force, const FVector& Location);
	FVector GetForce() const;

	void AddWorldTorque(const FVector& Torque);
	void AddCenterOfMassTorque(const FVector& Torque);
	FVector GetTorque() const;

	bool HasNative() const;
	void AllocateNative();
	FRigidBodyRef* GetNative();
	const FRigidBodyRef* GetNative() const;
	void ReleaseNative();

private:
	FRigidBodyBarrier(const FRigidBodyBarrier&) = delete;
	void operator=(const FRigidBodyBarrier&) = delete;

private:
	std::unique_ptr<FRigidBodyRef> NativeRef;
	FMassPropertiesBarrier MassProperties;
};
