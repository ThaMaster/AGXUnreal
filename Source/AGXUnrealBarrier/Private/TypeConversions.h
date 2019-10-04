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

inline agx::Real Convert(float V)
{
	return static_cast<agx::Real>(V);
}

inline agx::Real ConvertDistance(float V, const UWorld* World)
{
	return static_cast<agx::Real>(V * UnrealDistanceToAGXDistance(World));
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
