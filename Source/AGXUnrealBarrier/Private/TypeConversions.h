#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

#include "AGX_MotionControl.h"

#include <Math/Vector.h>

#include <agx/Vec3.h>
#include <agx/RigidBody.h>

inline float Convert(agx::Real V)
{
	return static_cast<float>(V);
}

inline agx::Real Convert(float V)
{
	return static_cast<agx::Real>(V);
}

inline FVector Convert(agx::Vec3 V)
{
	return FVector(Convert(V.x()), Convert(V.y()), Convert(V.z()));
}

inline agx::Vec3 Convert(FVector V)
{
	return agx::Vec3(Convert(V.X), Convert(V.Y), Convert(V.Z));
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
