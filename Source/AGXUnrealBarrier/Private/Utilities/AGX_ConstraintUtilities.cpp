#include "AGX_ConstraintUtilities.h"

#include "TypeConversions.h"
#include "AGXRefs.h"

namespace
{
	/**
	 * Given a Barrier, returns the final AGX native object.
	 */
	template <typename TNative, typename TBarrier>
	TNative* GetNativeFromBarrier(const TBarrier* Barrier)
	{
		if (Barrier && Barrier->HasNative())
			return Barrier->GetNative()->Native.get();
		else
			return nullptr;
	}
}

void FAGX_ConstraintUtilities::ConvertConstraintBodiesAndFrames(
	const FRigidBodyBarrier* RigidBody1, const FVector* FramePosition1, const FQuat* FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector* FramePosition2, const FQuat* FrameRotation2,
	agx::RigidBody*& NativeRigidBody1, agx::FrameRef& NativeFrame1,
	agx::RigidBody*& NativeRigidBody2, agx::FrameRef& NativeFrame2)
{
	// Convert first Rigid Body and Frame to natives
	{
		check(RigidBody1);
		check(FramePosition1);
		check(FrameRotation1);

		NativeRigidBody1 = GetNativeFromBarrier<agx::RigidBody>(RigidBody1);
		check(NativeRigidBody1);

		NativeFrame1 = ConvertFrame(*FramePosition1, *FrameRotation1);
	}

	// Convert second Rigid Body and Frame to natives
	{
		NativeRigidBody2 = GetNativeFromBarrier<agx::RigidBody>(RigidBody2);
		if (NativeRigidBody2)
		{
			check(FramePosition2);
			check(FrameRotation2);

			NativeFrame2 = ConvertFrame(*FramePosition2, *FrameRotation2);
		}
		else
		{
			NativeFrame2 = nullptr;
		}
	}
}
