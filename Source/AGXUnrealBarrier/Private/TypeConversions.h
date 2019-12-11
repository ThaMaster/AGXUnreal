#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

// AGXUnrealBarrierIncludes.
#include "AGX_MotionControl.h"
#include "AGXRefs.h"
#include "AGX_LogCategory.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include <LogVerbosity.h>
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/TwoVectors.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agx/Line.h>
#include <agx/Notify.h>
#include <agx/RigidBody.h>
#include <agx/Quat.h>
#include <agx/Vec3.h>
#include "EndAGXIncludes.h"
/// \note These functions assume that agx::Real and float are different types.
/// They also assume that agx::Real has higher (or equal) precision than float.

namespace
{
	template <typename T>
	constexpr T AGX_TO_UNREAL_DISTANCE_FACTOR = T(100.0);

	template <typename T>
	constexpr T UNREAL_TO_AGX_DISTANCE_FACTOR = T(0.01);
}

inline float Convert(agx::Real V)
{
	return static_cast<float>(V);
}

inline float ConvertDistance(agx::Real V)
{
	return static_cast<float>(V * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename T>
inline T ConvertDistanceToUnreal(agx::Real V)
{
	return static_cast<T>(V * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

inline agx::Real Convert(float V)
{
	return static_cast<agx::Real>(V);
}

inline agx::Real ConvertDistance(float V)
{
	return static_cast<agx::Real>(V) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename T>
inline agx::Real ConvertDistanceToAgx(T V)
{
	return static_cast<agx::Real>(V) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

inline FVector Convert(agx::Vec3 V)
{
	return FVector(Convert(V.x()), Convert(V.y()), Convert(V.z()));
}

inline FVector ConvertDistance(const agx::Vec3& V)
{
	return FVector(ConvertDistance(V.x()), ConvertDistance(V.y()), ConvertDistance(V.z()));
}

inline FVector ConvertVector(const agx::Vec3& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(ConvertDistance(V.x()), -ConvertDistance(V.y()), ConvertDistance(V.z()));
}

inline agx::Vec3 Convert(const FVector& V)
{
	return agx::Vec3(Convert(V.X), Convert(V.Y), Convert(V.Z));
}

inline agx::Vec3 ConvertDistance(const FVector& V)
{
	return agx::Vec3(ConvertDistance(V.X), ConvertDistance(V.Y), ConvertDistance(V.Z));
}

inline agx::Vec3 ConvertVector(const FVector& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return agx::Vec3(ConvertDistance(V.X), -ConvertDistance(V.Y), ConvertDistance(V.Z));
}

inline agx::Line Convert(const FTwoVectors& Vs)
{
	return {Convert(Vs.v1), Convert(Vs.v2)};
}

inline agx::Line ConvertDistance(const FTwoVectors& Vs)
{
	return {ConvertDistance(Vs.v1), Convert(Vs.v2)};
}

inline agx::Line ConvertVector(const FTwoVectors& Vs)
{
	return {ConvertVector(Vs.v1), Convert(Vs.v2)};
}

inline FQuat Convert(const agx::Quat& V)
{
	return FQuat(Convert(V.x()), -Convert(V.y()), Convert(V.z()), -Convert(V.w()));
}

inline agx::Quat Convert(const FQuat& V)
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

inline FString Convert(const agx::String& StringAGX)
{
	return FString(UTF8_TO_TCHAR(StringAGX.c_str()));
}

inline agx::String Convert(const FString& StringUnreal)
{
	return agx::String(TCHAR_TO_UTF8(*StringUnreal));
}

inline FGuid Convert(const agx::Uuid& Uuid)
{
	// Would like to use Uuid::size here, since that is the size of the data
	// pointed to by Uuid::data, but it's not constexpr.
	static_assert(sizeof(Uuid) == 4 * sizeof(uint32), "Unreal Guid and AGX Dynamics Uuid must be the same size.");
	uint32 Abcd[4];
	memcpy(Abcd, Uuid.data(), sizeof(Abcd));
	return FGuid(Abcd[0], Abcd[1], Abcd[2], Abcd[3]);
}

inline agx::Uuid Convert(const FGuid& Guid)
{
	uint32 Abcd[4];
	for (int i = 0; i < 4; ++i)
	{
		Abcd[i] = Guid[i];
	}
	agx::Uuid Uuid;
	memcpy(Uuid.data(), Abcd, Uuid.size());
	return Uuid;
}

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

inline agx::FrameRef ConvertFrame(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return new agx::Frame(agx::AffineMatrix4x4(Convert(FrameRotation), ConvertVector(FramePosition)));
}

/// \todo Consider moving this to the .cpp file.
inline void ConvertConstraintBodiesAndFrames(
	const FRigidBodyBarrier* RigidBody1, const FVector* FramePosition1, const FQuat* FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector* FramePosition2, const FQuat* FrameRotation2,
	agx::RigidBody*& NativeRigidBody1, agx::FrameRef& NativeFrame1, agx::RigidBody*& NativeRigidBody2,
	agx::FrameRef& NativeFrame2)
{
	// Convert first Rigid Body and Frame to natives
	{
		check(RigidBody1);
		check(FramePosition1);
		check(FrameRotation1);

		NativeRigidBody1 = GetNativeFromBarrier<agx::RigidBody>(RigidBody1);
		check(NativeRigidBody1);

		NativeFrame1 = ConvertFrame(*FramePosition1, *FrameRotation1);
		NativeRigidBody1->addAttachment(NativeFrame1, "ConstraintAttachment");
	}

	// Convert second Rigid Body and Frame to natives
	{
		NativeRigidBody2 = GetNativeFromBarrier<agx::RigidBody>(RigidBody2);
		if (NativeRigidBody2)
		{
			check(FramePosition2);
			check(FrameRotation2);

			NativeFrame2 = ConvertFrame(*FramePosition2, *FrameRotation2);
			NativeRigidBody2->addAttachment(NativeFrame2, "ConstraintAttachment");
		}
		else
		{
			NativeFrame2 = nullptr;
		}
	}
}

inline agx::Notify::NotifyLevel ConvertLogLevelVerbosity(ELogVerbosity::Type LogVerbosity)
{
	switch (LogVerbosity)
	{
		case ELogVerbosity::VeryVerbose:
			return agx::Notify::NOTIFY_DEBUG;
		case ELogVerbosity::Verbose:
			return agx::Notify::NOTIFY_DEBUG;
		case ELogVerbosity::Log:
			return agx::Notify::NOTIFY_INFO;
		case ELogVerbosity::Display:
			return agx::Notify::NOTIFY_WARNING;
		case ELogVerbosity::Warning:
			return agx::Notify::NOTIFY_WARNING;
		case ELogVerbosity::Error:
			return agx::Notify::NOTIFY_ERROR;
		case ELogVerbosity::Fatal:
			return agx::Notify::NOTIFY_ERROR;
		default:
			UE_LOG(
				LogAGX, Warning,
				TEXT("ConvertLogLevelVerbosity: unknown verbosity level: %d. Verbosity level 'NOTIFY_INFO' will be used instead."),
				LogVerbosity);

			// Use NOTIFY_INFO as default, if unknown log verbosity is given
			return agx::Notify::NOTIFY_INFO;
	}
}
