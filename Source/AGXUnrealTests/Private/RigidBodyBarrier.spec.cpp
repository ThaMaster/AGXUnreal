
// AGXUnreal includes.
#include "RigidBodyBarrier.h"
#include "TestHelpers.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

BEGIN_DEFINE_SPEC(
	FRigidBodyBarrierSpec, "AGXUnreal.Barrier.RigidBody", TestHelpers::DefaultTestFlags)
END_DEFINE_SPEC(FRigidBodyBarrierSpec)

void FRigidBodyBarrierSpec::Define()
{
	Describe("Allocating and releasing native", [this]() {
		It("should be created without native", [this]() {
			FRigidBodyBarrier RigidBody;
			TestFalse("Barrier should be created without native.", RigidBody.HasNative());
		});

		It("should have a native after allocation", [this]() {
			FRigidBodyBarrier RigidBody;
			RigidBody.AllocateNative();
			TestTrue("Allocation should give the barrier a native.", RigidBody.HasNative());
			TestNotNull(
				"Allocation should give the barrier a non-null native", RigidBody.GetNative());
		});

		It("should not have a native after release", [this]() {
			FRigidBodyBarrier RigidBody;
			RigidBody.AllocateNative();
			RigidBody.ReleaseNative();
			TestFalse("Release should remove the native from the barrier.", RigidBody.HasNative());
		});
	});

	Describe("Setting and getting properties", [this]() {

	});
}
