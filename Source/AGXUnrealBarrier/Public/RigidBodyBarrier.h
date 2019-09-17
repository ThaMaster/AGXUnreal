#pragma once

#include <Math/Vector.h>

#include <memory>


struct FRigidBodyRef;

/**
 * Barrier between UAGX_RigidBody and agx::RigidBody. UAGX_RigidBody holds an
 * instance of RigidBodyBarrier and hidden behind the RigidBodyBarrier is a
 * agx::RigidBodyRef. This allows UAGX_RigidBody to interacti with
 * agx::RigidBody without including agx/RigidBody.h.
 *
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
class FRigidBodyBarrier
{
public:
	FRigidBodyBarrier();
	~FRigidBodyBarrier();

	void SetPosition(FVector NewPosition);
	FVector GetPosition() const;

	void SetMass(float NewMass);
	float GetMass();

	bool HasNative() const;
	void AllocateNative();

	void DebugSimulate();

private:
	FRigidBodyBarrier(const FRigidBodyBarrier&) = delete;
	void operator=(const FRigidBodyBarrier&) = delete;

private:
	std::unique_ptr<FRigidBodyRef> NativeRef;
};

