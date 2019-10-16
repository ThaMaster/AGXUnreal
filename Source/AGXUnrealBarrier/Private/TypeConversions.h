#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

#include "AGX_MotionControl.h"

#include <Math/Vector.h>
#include <Math/Quat.h>
#include <Engine/World.h>
#include <GameFramework/WorldSettings.h>

#include <agx/Vec3.h>
#include <agx/Quat.h>
#include <agx/RigidBody.h>

#include "RigidBodyBarrier.h"


/// \note These functions assume that agx::Real and float are different types.

/// \return The number of meters in the AGX Dynamics simulation an Unreal
///         distance of 1 corresponds to. Unreal distances are often in
///         centimeters and in that case UnrealDistanceToAGXDistance would
///         return 0.01.
///         Note that this is the inverse of Unreal's WorldToMeters.
inline float UnrealDistanceToAGXDistance(const UWorld* World)
{
	/// \todo WorldToMeters was 'static' for a while, for performance reasons,
	///       but this leads to bugs when the world scale is changed between
	///       simulations. I expect the cost of getting the scale to be small,
	///       until the opposite has been proven. But this function is
	///       potentially called many times during BeginPlay which is and
	///       operation that we really want to be as fast as possible.
	float WorldToMeters = World->GetWorldSettings()->WorldToMeters;
	return 1.0f / WorldToMeters;
}

inline float AGXDistanceToUnrealDistance(const UWorld* World)
{
	float WorldToMeters = World->GetWorldSettings()->WorldToMeters;
	return WorldToMeters;
}

inline float Convert(agx::Real V)
{
	return static_cast<float>(V);
}

inline float ConvertDistance(agx::Real V, const UWorld* World)
{
	return static_cast<float>(V) * AGXDistanceToUnrealDistance(World);
}

inline double ConvertDistanceToUnreal(agx::Real V, const UWorld* World)
{
	return static_cast<double>(V) * double(AGXDistanceToUnrealDistance(World));
}

inline float ConvertDistanceToUnrealF(agx::Real V, const UWorld* World)
{
	return static_cast<float>(V) * float(AGXDistanceToUnrealDistance(World));
}

inline agx::Real Convert(float V)
{
	return static_cast<agx::Real>(V);
}

inline agx::Real ConvertDistance(float V, const UWorld* World)
{
	return static_cast<agx::Real>(V * UnrealDistanceToAGXDistance(World));
}

inline agx::Real ConvertDistanceToAgx(double V, const UWorld* World)
{
	return static_cast<agx::Real>(V * double(UnrealDistanceToAGXDistance(World)));
}

inline agx::Real ConvertDistanceToAgx(float V, const UWorld* World)
{
	return static_cast<agx::Real>(V * float(UnrealDistanceToAGXDistance(World)));
}

inline FVector Convert(agx::Vec3 V)
{
	return FVector(Convert(V.x()), Convert(V.y()), Convert(V.z()));
}

inline FVector ConvertDistance(const agx::Vec3 &V, const UWorld* World)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(ConvertDistance(V.x(), World), -ConvertDistance(V.y(), World), ConvertDistance(V.z(), World));
}

inline agx::Vec3 Convert(const FVector &V)
{
	return agx::Vec3(Convert(V.X), Convert(V.Y), Convert(V.Z));
}

inline agx::Vec3 ConvertDistance(const FVector &V, const UWorld* World)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return agx::Vec3(ConvertDistance(V.X, World), -ConvertDistance(V.Y, World), ConvertDistance(V.Z, World));
}

inline FQuat Convert(const agx::Quat &V)
{
	return FQuat(Convert(V.x()), -Convert(V.y()), Convert(V.z()), -Convert(V.w()));
}

inline agx::Quat Convert(const FQuat &V)
{
	return agx::Quat(Convert(V.X), -Convert(V.Y), Convert(V.Z), -Convert(V.W));
}

inline agx::RigidBody::MotionControl Convert(EAGX_MotionControl V)
{
	switch (V)
	{
		case MC_STATIC:
			return agx::RigidBody::STATIC;
		case MC_KINEMATICS:
			return agx::RigidBody::KINEMATICS;
		case MC_DYNAMICS:
			return agx::RigidBody::DYNAMICS;
	}
	/// \todo Add UE_LOG(LogAGX, ...) here.
	return agx::RigidBody::DYNAMICS;
}

inline EAGX_MotionControl Convert(agx::RigidBody::MotionControl V)
{
	switch (V)
	{
		case agx::RigidBody::STATIC:
			return MC_STATIC;
		case agx::RigidBody::KINEMATICS:
			return MC_KINEMATICS;
		case agx::RigidBody::DYNAMICS:
			return MC_DYNAMICS;
	}
	/// \todo Add UE_LOG(LogAGX, ...) here.
	return MC_KINEMATICS;
}

/**
 * Given a Barrier, returns the final AGX native object.
 */
template<typename TNative, typename TBarrier>
TNative* GetNativeFromBarrier(const TBarrier* Barrier)
{
	if (Barrier && Barrier->HasNative())
		return Barrier->GetNative()->Native.get();
	else
		return nullptr;
}

inline agx::FrameRef ConvertFrame(const FVector& FramePosition, const FQuat& FrameRotation, const UWorld* World)
{
	return new agx::Frame(
		agx::AffineMatrix4x4(
			Convert(FrameRotation),
			ConvertDistance(FramePosition, World)));
}

/// \todo Consider moving this to the .cpp file.
inline void ConvertConstraintBodiesAndFrames(
	const FRigidBodyBarrier* RigidBody1, const FVector* FramePosition1, const FQuat* FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector* FramePosition2, const FQuat* FrameRotation2,
	const UWorld* World,
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

		NativeFrame1 = ConvertFrame(*FramePosition1, *FrameRotation1, World);
		NativeRigidBody1->addAttachment(NativeFrame1, "ConstraintAttachment");
	}

	// Convert second Rigid Body and Frame to natives
	{
		NativeRigidBody2 = GetNativeFromBarrier<agx::RigidBody>(RigidBody2);
		if (NativeRigidBody2)
		{
			check(FramePosition2);
			check(FrameRotation2);

			NativeFrame2 = ConvertFrame(*FramePosition2, *FrameRotation2, World);
			NativeRigidBody2->addAttachment(NativeFrame2, "ConstraintAttachment");
		}
		else
		{
			NativeFrame2 = nullptr;
		}
	}
}
