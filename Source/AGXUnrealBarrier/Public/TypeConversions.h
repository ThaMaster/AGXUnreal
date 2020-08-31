#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

// AGXUnreal includes.
#include "AGX_MotionControl.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Logging/LogVerbosity.h"
#include "Math/Interval.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/Quat.h"
#include "Math/TwoVectors.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/Line.h>
#include <agx/Notify.h>
#include <agx/RigidBody.h>
#include <agx/Quat.h>
#include <agx/Vec2.h>
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

// Scalars.

inline float Convert(agx::Real V)
{
	return static_cast<float>(V);
}

inline float ConvertDistance(agx::Real D)
{
	return static_cast<float>(D * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename T>
inline T ConvertDistanceToUnreal(agx::Real D)
{
	return static_cast<T>(D * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

inline float ConvertAngle(agx::Real A)
{
	return Convert(FMath::RadiansToDegrees(A));
}

template <typename T>
inline T ConvertAngleToUnreal(agx::Real A)
{
	return static_cast<T>(FMath::RadiansToDegrees(A));
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
inline agx::Real ConvertDistanceToAgx(T D)
{
	return static_cast<agx::Real>(D) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

inline agx::Real ConvertAngle(float A)
{
	return FMath::DegreesToRadians(Convert(A));
}

template <typename T>
inline agx::Real ConvertAngleToAgx(T A)
{
	// Precision pessimization here because of 32-bit pi in FMath?
	return FMath::DegreesToRadians(A);
}

// Two-dimensional vectors.

inline FVector2D Convert(const agx::Vec2& V)
{
	return FVector2D(Convert(V.x()), Convert(V.y()));
}

inline FVector2D ConvertDistance(const agx::Vec2& V)
{
	return FVector2D(ConvertDistance(V.x()), ConvertDistance(V.y()));
}

// No ConvertVector for two-dimensional vectors because there is no handedness here, so it would be
// identical to ConvertDistance.

inline agx::Vec2 Convert(const FVector2D& V)
{
	return agx::Vec2(Convert(V.X), Convert(V.Y));
}

inline agx::Vec2 ConvertDistance(const FVector2D& V)
{
	return agx::Vec2(ConvertDistance(V.X), ConvertDistance(V.Y));
}

// Three-dimensional vectors.

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

inline FVector ConvertAngularVelocity(const agx::Vec3& V)
{
	/*
	 * Angular velocities in Unreal are weird. Even rotations are kind of weird. We're basing this
	 * conversion on the rotation widget in the Details Panel. Unreal Engine uses a left-handed
	 * coordinate system, meaning that thumb=X, index=Y, middle=Z matches the left hand. Normally,
	 * rotations also has a handedness. Imagine gripping the axis around which we rotate with your
	 * thumb pointing towards increasing axis values and look at your (usually) four non-thumb
	 * fingers. Their direction from the knuckles towards the finger tips define the direction of
	 * positive rotation. If you switch hand then the direction of positive rotation is inverted. In
	 * Unreal Engine, at least according to the rotation widget in the Details Panel, uses
	 * right-handed rotations for the X and Y axes, and left-handed rotations for the Z axis.
	 *
	 * AGX Dynamics is right-handed throughout. There are two sets of flips going on, one because of
	 * the left-vs-right-handedness of the coordinate system itself and one for the
	 * left-vs-right-handedness of each axis' rotation. The X axis point in the same direction in
	 * both cases and is right-handed in both cases, so we pass it through untouched. The Y axis
	 * should be negated because of the right-to-left switch of the coordinate system, but the
	 * rotations are right-handed in both cases so one negation is enough. The Z axis point in the
	 * same direction in both cases, so no negation there, but the handedness of rotations around X
	 * is different so so we must negate it for that reason.
	 */
	return FVector(Convert(V.x()), -Convert(V.y()), -Convert(V.z()));
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

inline agx::Vec3 ConvertAngularVelocity(const FVector& V)
{
	// See comment in the AGX-to-Unreal version of this function.
	return agx::Vec3(Convert(V.X), -Convert(V.Y), -Convert(V.Z));
}

// Interval/Range.

inline FFloatInterval Convert(const agx::RangeReal& R)
{
	return FFloatInterval(Convert(R.lower()), Convert(R.upper()));
}

inline FFloatInterval ConvertDistance(const agx::RangeReal& R)
{
	return FFloatInterval(ConvertDistance(R.lower()), ConvertDistance(R.upper()));
}

inline FFloatInterval ConvertAngle(const agx::RangeReal& R)
{
	return FFloatInterval(ConvertAngle(R.lower()), ConvertAngle(R.upper()));
}

inline agx::RangeReal Convert(const FFloatInterval& I)
{
	return agx::RangeReal(Convert(I.Min), Convert(I.Max));
}

inline agx::RangeReal ConvertDistance(const FFloatInterval& I)
{
	return agx::RangeReal(ConvertDistance(I.Min), ConvertDistance(I.Max));
}

inline agx::RangeReal ConvertAngle(const FFloatInterval& I)
{
	return agx::RangeReal(ConvertAngle(I.Min), ConvertAngle(I.Max));
}

// TwoVectors/Line.
// TwoVectors may represent other things as well. If that's the case then we'll
// need to do something else.

inline agx::Line Convert(const FTwoVectors& Vs)
{
	return {Convert(Vs.v1), Convert(Vs.v2)};
}

inline agx::Line ConvertDistance(const FTwoVectors& Vs)
{
	return {ConvertDistance(Vs.v1), ConvertDistance(Vs.v2)};
}

inline agx::Line ConvertVector(const FTwoVectors& Vs)
{
	return {ConvertVector(Vs.v1), ConvertVector(Vs.v2)};
}

// Quaternions.

inline FQuat Convert(const agx::Quat& V)
{
	return FQuat(Convert(V.x()), -Convert(V.y()), Convert(V.z()), -Convert(V.w()));
}

inline agx::Quat Convert(const FQuat& V)
{
	return agx::Quat(Convert(V.X), -Convert(V.Y), Convert(V.Z), -Convert(V.W));
}

// Text.

inline FString Convert(const agx::String& StringAGX)
{
	return FString(UTF8_TO_TCHAR(StringAGX.c_str()));
}

inline FString Convert(const agx::Name& NameAGX)
{
	// For some reason agx::Name seems to sometimes give runtime crashes when copied. Therefore,
	// agx::Name.c_str() is used, and copying the type in general should be avoided. The underlying
	// reason for this is not yet known.
	return FString(NameAGX.c_str());
}

inline agx::String Convert(const FString& StringUnreal)
{
	return agx::String(TCHAR_TO_UTF8(*StringUnreal));
}

inline agx::Name Convert(const FName& NameUnreal)
{
	return agx::Name(TCHAR_TO_UTF8(*(NameUnreal.ToString())));
}

inline FGuid Convert(const agx::Uuid& Uuid)
{
	// Would like to use Uuid::size here, since that is the size of the data
	// pointed to by Uuid::data, but it's not constexpr.
	static_assert(
		sizeof(Uuid) == 4 * sizeof(uint32),
		"Unreal Guid and AGX Dynamics Uuid must be the same size.");
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

// Enumerations.

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

inline agx::Constraint2DOF::DOF Convert(EAGX_Constraint2DOFFreeDOF Dof)
{
	check(Dof == EAGX_Constraint2DOFFreeDOF::FIRST || Dof == EAGX_Constraint2DOFFreeDOF::SECOND);

	return Dof == EAGX_Constraint2DOFFreeDOF::FIRST ? agx::Constraint2DOF::FIRST
													: agx::Constraint2DOF::SECOND;
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
				TEXT("ConvertLogLevelVerbosity: unknown verbosity level: %d. Verbosity level "
					 "'NOTIFY_INFO' will be used instead."),
				LogVerbosity);

			// Use NOTIFY_INFO as default, if unknown log verbosity is given
			return agx::Notify::NOTIFY_INFO;
	}
}

inline agx::FrameRef ConvertFrame(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return new agx::Frame(
		agx::AffineMatrix4x4(Convert(FrameRotation), ConvertVector(FramePosition)));
}

inline uint32 StringTo32BitFnvHash(const FString& StringUnreal)
{
	/// \todo Verify that we get the same hash as AGX Dynamics for Unity.
	/// I worry that `TCHAR`, which is 16 bit, will give a different result
	/// compared to the C# `byte` type.
	auto bytes = StringUnreal.GetCharArray();

	uint32 hash = 2166136261U;

	for (auto& singleByte : bytes)
	{
		hash ^= singleByte;
		hash *= 16777619U;
	}

	return hash;
}
