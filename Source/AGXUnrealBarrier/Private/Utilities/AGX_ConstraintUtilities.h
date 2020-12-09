#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Quat.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include "EndAGXIncludes.h"

class AGXUNREALBARRIER_API FAGX_ConstraintUtilities
{
public:
	static void ConvertConstraintBodiesAndFrames(
		const FRigidBodyBarrier* RigidBody1, const FVector* FramePosition1,
		const FQuat* FrameRotation1, const FRigidBodyBarrier* RigidBody2,
		const FVector* FramePosition2, const FQuat* FrameRotation2,
		agx::RigidBody*& NativeRigidBody1, agx::FrameRef& NativeFrame1,
		agx::RigidBody*& NativeRigidBody2, agx::FrameRef& NativeFrame2);
};
