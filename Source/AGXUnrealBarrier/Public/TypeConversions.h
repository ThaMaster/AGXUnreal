// Copyright 2022, Algoryx Simulation AB.

#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"
#include "AGX_LogCategory.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Materials/AGX_ContactMaterialEnums.h"
#include "RigidBodyBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/DoubleInterval.h"
#include "Vehicle/AGX_TrackEnums.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Logging/LogVerbosity.h"
#include "Math/Interval.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/Quat.h"
#include "Math/TwoVectors.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/FrictionModel.h>
#include <agx/Line.h>
#include <agx/Notify.h>
#include <agx/RigidBody.h>
#include <agx/Quat.h>
#include <agx/Vec2.h>
#include <agx/Vec3.h>
#include <agxModel/TwoBodyTire.h>
#include <agxVehicle/TrackInternalMergeProperties.h>
#include <agxVehicle/TrackWheel.h>
#include <agxWire/Node.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <limits>

// These functions assume that agx::Real and float are different types.
// They also assume that agx::Real has higher (or equal) precision than float.
// We support both float and double on the Unreal Engine side. Function name
// suffixes are used where necessary to disambiguate between return types.
//
// Naming conventions:
//
// Convert
//
// The default conversion function is named Convert. It is overloaded on the parameter type and
// detects if it is an AGX Dynamics type or an Unreal Engine type. double is considered an AGX
// Dynamics type and float an Unreal Engine type. It converts to the other. The conversion is just a
// cast, a plain Convert will never do any unit translations. It can do range checks, which when
// failed will result in an error message being printed and the value truncated. Composite types,
// such as Vector, calls Convert on its members.
//
//
// ConvertDistance
//
// Acts like Convert except that AGX Dynamics types are multiplied by 100 before being converted to
// the corresponding Unreal Engine type, and Unreal Engine types are divided by 100 after being
// converted to the AGX Dynamics type. The unit conversion is always performed using the AGX
// Dynamics types because we assume that agx::Real is at least as precise as float.
//
//
// ConvertAngle
//
// Text... Degrees/radians.
//
//
// Convert<UNIT>ToUnreal / Convert<UNIT>ToAgx
//
// Perform the same operation as Convert<UNIT>, where unit can be e.g., Distance or Angle, but with
// caller control over the return type. Used, for example, when we want to convert from an AGX
// Dynamics unit to an Unreal Engine unit but want the result as a double instead of a float. Also
// used when we have a value in one unit-space stored in the other type-space, e.g., an AGX Dynamics
// distance in an Unreal Engine type.
//
//
// ConvertUNITFloat
//
// The Float-suffix is added when the parameter type based overload produces the correct unit
// conversion but where the default conversion would produce a double, or double composite type, but
// we need a float, or a float composite.

template <typename T>
constexpr T AGX_TO_UNREAL_DISTANCE_FACTOR = T(100.0);

template <typename T>
constexpr T UNREAL_TO_AGX_DISTANCE_FACTOR = T(0.01);

//
// Scalars. AGX Dynamics to Unreal Engine.
//
// We provide untyped (not distance, angle, etc) scalar conversion functions
// because the naive Unreal=float AGX=double isn't true since Unreal Engine 5.
// 'float Convert(double)' and 'double Convert(float)' would be dangerous
// because in some circumstances, when a double is passed from Unreal Engine to
// AGX Dynamics, that could lead to a double -> float -> double round-trip
// through implicit type conversions. We therefore provide templated
// ConvertToUnreal and ConvertToAGX functions. The templating control the Unreal
// type and the AGX type is always agx::Real. So for ConvertToAGX the parameter
// is templated and for ConvertToUnreal the return value is templated.
//

static_assert(
	std::numeric_limits<agx::Real>::max() >= std::numeric_limits<float>::max(),
	"Expecting agx::Real to hold all values that float can hold.");

static_assert(
	std::numeric_limits<agx::Int>::max() >= std::numeric_limits<int32>::max(),
	"Expecting agx::Int to hold all positive values that int32 can hold.");

static_assert(
	std::numeric_limits<std::size_t>::max() >= std::numeric_limits<int32>::max(),
	"Expecting std::size_t to hold all positive values that int32 can hold.");

template <typename TU>
inline TU ConvertToUnreal(agx::Real D)
{
	return static_cast<TU>(D);
}

template <typename TU>
inline TU ConvertDistanceToUnreal(agx::Real D)
{
	return static_cast<TU>(D * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertAreaToUnreal(agx::Real D2)
{
	return static_cast<TU>(
		D2 * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real> * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertDistanceInvToUnreal(agx::Real DInv)
{
	return static_cast<TU>(DInv / AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertAngleToUnreal(agx::Real A)
{
	return static_cast<TU>(FMath::RadiansToDegrees(A));
}

inline int32 Convert(agx::Int I)
{
	static constexpr agx::Int MaxAllowed = static_cast<agx::Int>(std::numeric_limits<int32>::max());
	if (I > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large agx::Int being converted to int32, value is truncated."));
		I = MaxAllowed;
	}
	return static_cast<int32>(I);
}

inline int32 Convert(std::size_t S)
{
	static constexpr std::size_t MaxAllowed =
		static_cast<std::size_t>(std::numeric_limits<int32>::max());
	if (S > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large size_t being converted to int32, value is truncated."));
		S = MaxAllowed;
	}
	return static_cast<int32>(S);
}

//
// Scalars. Unreal Engine to AGX Dynamics.
//

template <typename TU>
inline agx::Real ConvertToAGX(TU D)
{
	return static_cast<agx::Real>(D);
}

template <typename TU>
inline agx::Real ConvertDistanceToAGX(TU D)
{
	return static_cast<agx::Real>(D) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertAreaToAGX(TU D2)
{
	return static_cast<agx::Real>(D2) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real> *
		   UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertDistanceInvToAGX(TU DInv)
{
	return static_cast<agx::Real>(DInv) / UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertAngleToAGX(TU A)
{
	return FMath::DegreesToRadians(static_cast<agx::Real>(A));
}

inline agx::Int Convert(int32 I)
{
	return static_cast<agx::Int>(I);
}

//
// Two-dimensional vectors. AGX Dynamics to Unreal Engine.
//

inline FVector2D Convert(const agx::Vec2& V)
{
	return FVector2D(
		ConvertToUnreal<decltype(FVector2D::X)>(V.x()),
		ConvertToUnreal<decltype(FVector2D::X)>(V.y()));
}

inline FVector2D ConvertDistance(const agx::Vec2& V)
{
	return FVector2D(
		ConvertDistanceToUnreal<decltype(FVector2D::X)>(V.x()),
		ConvertDistanceToUnreal<decltype(FVector2D::X)>(V.y()));
}

// No ConvertVector for two-dimensional vectors because there is no handedness here, so it would be
// identical to ConvertDistance.

//
// Two-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec2 Convert(const FVector2D& V)
{
	return agx::Vec2(ConvertToAGX(V.X), ConvertToAGX(V.Y));
}

inline agx::Vec2 ConvertDistance(const FVector2D& V)
{
	return agx::Vec2(ConvertDistanceToAGX(V.X), ConvertDistanceToAGX(V.Y));
}

//
// Three-dimensional vectors. AGX Dynamics to Unreal Engine.
//

/*
 * There are a few different cases here, characterized by whether we convert cm <> m and
 * whether we flip the Y axis, since Unreal Engine is left-handed and AGX Dynamics is
 * right-handed.
 *
 *             Convert cm <> m
 *       |     No    |    Yes       |
 *     --|-----------|--------------|
 *   F N |           | Convert      |
 *   l o | Convert   | Distance     |
 *   i   |           |              |
 *   p --|-----------|--------------|
 *     Y | Convert   | Convert      |
 *   Y e | Vector    | Displacement |
 *     s |           |              |
 *     --|-----------|--------------|
 *
 *
 * Angular velocity is a beast of its own with a big comment all to itself.
 */

inline FVector Convert(const agx::Vec3& V)
{
	return FVector(
		ConvertToUnreal<decltype(FVector::X)>(V.x()), ConvertToUnreal<decltype(FVector::X)>(V.y()),
		ConvertToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertDistance(const agx::Vec3& V)
{
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertVector(const agx::Vec3& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(
		ConvertToUnreal<decltype(FVector::X)>(V.x()), -ConvertToUnreal<decltype(FVector::X)>(V.y()),
		ConvertToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertDisplacement(const agx::Vec3& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertFloatVector(const agx::Vec3f& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertAngularVelocity(const agx::Vec3& V)
{
	/*
	 * Angular velocities in Unreal are weird. Even rotations are kind of weird. We're basing this
	 * conversion on the rotation widget in the Details Panel. Unreal Engine uses a left-handed
	 * coordinate system, meaning that thumb=X, index=Y, middle=Z matches the left hand. Normally,
	 * rotations also has a handedness. Imagine gripping the axis around which we rotate with your
	 * thumb pointing towards increasing axis values and look at your (usually) four non-thumb
	 * fingers. Their direction from the knuckles towards the fingertips define the direction of
	 * positive rotation. If you switch hand then the direction of positive rotation is inverted.
	 * Unreal Engine, at least according to the rotation widget in the Details Panel, uses
	 * right-handed rotations for the X and Y axes, and left-handed rotations for the Z axis.
	 *
	 * AGX Dynamics is right-handed throughout. There are two sets of flips going on, one because of
	 * the left-vs-right-handedness of the coordinate system itself and one for the
	 * left-vs-right-handedness of each axis' rotation. The X axis point in the same direction in
	 * both cases and is right-handed in both cases, so we pass it through untouched. The Y axis
	 * should be negated because of the right-to-left switch of the coordinate system, but the
	 * rotations are right-handed in both cases so one negation is enough. The Z axis point in the
	 * same direction in both cases, so no negation there, but the handedness of rotations around Z
	 * is different so we must negate it for that reason.
	 */
	return FVector(
		FMath::RadiansToDegrees(ConvertToUnreal<decltype(FVector::X)>(V.x())),
		FMath::RadiansToDegrees(-ConvertToUnreal<decltype(FVector::X)>(V.y())),
		FMath::RadiansToDegrees(-ConvertToUnreal<decltype(FVector::X)>(V.z())));
}

inline FVector ConvertTorque(const agx::Vec3& V)
{
	/*
	 * Following a similar logic as ConvertAngularVelocity for the axis directions, but no unit
	 * conversion since we use Nm in both AGX Dynamics and Unreal Engine.
	 */
	// clang-format off
	return {
		ConvertToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertToUnreal<decltype(FVector::Y)>(V.y()),
		-ConvertToUnreal<decltype(FVector::Z)>(V.z())};
	// clang-format on
}

//
// Three-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec3 Convert(const FVector& V)
{
	return agx::Vec3(ConvertToAGX(V.X), ConvertToAGX(V.Y), ConvertToAGX(V.Z));
}

inline agx::Vec3 ConvertDistance(const FVector& V)
{
	return agx::Vec3(
		ConvertDistanceToAGX(V.X), ConvertDistanceToAGX(V.Y), ConvertDistanceToAGX(V.Z));
}

inline agx::Vec3 ConvertVector(const FVector& V)
{
	return agx::Vec3(ConvertToAGX(V.X), -ConvertToAGX(V.Y), ConvertToAGX(V.Z));
}

inline agx::Vec3 ConvertDisplacement(const FVector& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return agx::Vec3(
		ConvertDistanceToAGX(V.X), -ConvertDistanceToAGX(V.Y), ConvertDistanceToAGX(V.Z));
}

inline agx::Vec3f ConvertFloatVector(const FVector& V)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return agx::Vec3f(
		ConvertDistanceToAGX(V.X), -ConvertDistanceToAGX(V.Y), ConvertDistanceToAGX(V.Z));
}

inline agx::Vec3 ConvertAngularVelocity(const FVector& V)
{
	// See comment in the AGX-to-Unreal version of this function.
	return agx::Vec3(
		ConvertToAGX(FMath::DegreesToRadians(V.X)), -ConvertToAGX(FMath::DegreesToRadians(V.Y)),
		-ConvertToAGX(FMath::DegreesToRadians(V.Z)));
}

inline agx::Vec3 ConvertTorque(const FVector& V)
{
	/*
	 * Following a similar logic as ConvertAngularVelocity for the axis directions, but no unit
	 * conversion since we use Nm in both AGX Dynamics and Unreal Engine.
	 */
	return {ConvertToAGX(V.X), -ConvertToAGX(V.Y), -ConvertToAGX(V.Z)};
}

//
// Four-dimensional vectors. AGX Dynamics to Unreal Engine.
//

inline FVector4 Convert(const agx::Vec4& V)
{
	return FVector4(
		ConvertToUnreal<decltype(FVector4::X)>(V.x()),
		ConvertToUnreal<decltype(FVector4::X)>(V.y()),
		ConvertToUnreal<decltype(FVector4::X)>(V.z()),
		ConvertToUnreal<decltype(FVector4::X)>(V.w()));
}

inline FVector4 Convert(const agx::Vec4f& V)
{
	return FVector4(
		ConvertToUnreal<decltype(FVector4::X)>(V.x()),
		ConvertToUnreal<decltype(FVector4::X)>(V.y()),
		ConvertToUnreal<decltype(FVector4::X)>(V.z()),
		ConvertToUnreal<decltype(FVector4::X)>(V.w()));
}

//
// Four-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec4 Convert(const FVector4& V)
{
	return agx::Vec4(
		ConvertToAGX<decltype(FVector4::X)>(V.X), ConvertToAGX<decltype(FVector4::X)>(V.Y),
		ConvertToAGX<decltype(FVector4::X)>(V.Z), ConvertToAGX<decltype(FVector4::X)>(V.W));
}

inline agx::Vec4f ConvertFloat(const FVector4& V)
{
	return agx::Vec4f((float) V.X, (float) V.Y, (float) V.Z, (float) V.W);
}

//
// Interval/Range. AGX Dynamics to Unreal Engine.
//
// We've had some issues with the Interval classes built into Unreal Engine. Partly because they
// were float-only in Unreal Engine 4 (not sure about 5 yet), party because Unreal Engine 4.27
// crashes whenever an FFloatInterval Property containing infinity is displayed in a Details panel,
// and partly because they don't support scientific notation in the Details panel.
//
// Because of these we introduced FAGX_DoubleInterval and then FAGX_RealInterval.
//

inline FAGX_RealInterval Convert(const agx::RangeReal& R)
{
	return FAGX_RealInterval {R.lower(), R.upper()};
}

inline FAGX_RealInterval ConvertDistance(const agx::RangeReal& R)
{
	return FAGX_RealInterval {
		ConvertDistanceToUnreal<double>(R.lower()), ConvertDistanceToUnreal<double>(R.upper())};
}

inline FAGX_RealInterval ConvertAngle(const agx::RangeReal& R)
{
	return FAGX_RealInterval {
		ConvertAngleToUnreal<double>(R.lower()), ConvertAngleToUnreal<double>(R.upper())};
}

//
// Interval/Range. Unreal Engine to AGX Dynamics.
//

inline agx::RangeReal Convert(const FAGX_RealInterval& I)
{
	return agx::RangeReal(I.Min, I.Max);
}

inline agx::RangeReal ConvertDistance(const FAGX_RealInterval& I)
{
	return agx::RangeReal(ConvertDistanceToAGX(I.Min), ConvertDistanceToAGX(I.Max));
}

inline agx::RangeReal ConvertAngle(const FAGX_RealInterval& I)
{
	return agx::RangeReal(ConvertAngleToAGX(I.Min), ConvertAngleToAGX(I.Max));
}

//
// TwoVectors/Line. Unreal Engine to AGX Dynamics
// TwoVectors may represent other things as well. If that's the case then we'll
// need to do something else.
//

inline agx::Line Convert(const FTwoVectors& Vs)
{
	return {Convert(Vs.v1), Convert(Vs.v2)};
}

inline agx::Line ConvertDistance(const FTwoVectors& Vs)
{
	return {ConvertDistance(Vs.v1), ConvertDistance(Vs.v2)};
}

inline agx::Line ConvertDisplacement(const FTwoVectors& Vs)
{
	return {ConvertDisplacement(Vs.v1), ConvertDisplacement(Vs.v2)};
}

//
// Quaternions.
//

inline FQuat Convert(const agx::Quat& V)
{
	return FQuat(
		ConvertToUnreal<decltype(FQuat::X)>(V.x()), -ConvertToUnreal<decltype(FQuat::X)>(V.y()),
		ConvertToUnreal<decltype(FQuat::X)>(V.z()), -ConvertToUnreal<decltype(FQuat::X)>(V.w()));
}

inline agx::Quat Convert(const FQuat& V)
{
	return agx::Quat(
		ConvertToAGX<decltype(FQuat::X)>(V.X), -ConvertToAGX<decltype(FQuat::X)>(V.Y),
		ConvertToAGX<decltype(FQuat::X)>(V.Z), -ConvertToAGX<decltype(FQuat::X)>(V.W));
}

//
// Transformations.
//

inline FTransform Convert(const agx::AffineMatrix4x4& T)
{
	const FVector Translation = ConvertDisplacement(T.getTranslate());
	const FQuat Rotation = Convert(T.getRotate());
	return FTransform(Rotation, Translation);
}

inline agx::FrameRef ConvertFrame(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return new agx::Frame(
		agx::AffineMatrix4x4(Convert(FrameRotation), ConvertDisplacement(FramePosition)));
}

inline FTransform ConvertLocalFrame(const agx::Frame* Frame)
{
	return FTransform(
		Convert(Frame->getLocalRotate()), ConvertDisplacement(Frame->getLocalTranslate()));
}

inline agx::AffineMatrix4x4 ConvertMatrix(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return agx::AffineMatrix4x4(Convert(FrameRotation), ConvertDisplacement(FramePosition));
}

//
// Text.
//

inline FString Convert(const agx::String& StringAGX)
{
	return FString(UTF8_TO_TCHAR(StringAGX.c_str()));
}

inline FString Convert(const agx::Name& NameAGX)
{
	// Due to different memory allocators it is not safe to copy an agx::Name between AGX Dynamics
	// and Unreal Engine shared libraries, or to move ownership of the underlying memory buffer. By
	// passing the return value of c_str() to the FString constructor we create a copy of the
	// characters within the Unreal Engine shared library that is owned by that shared library,
	// and the AGX Dynamics agx::Name retain ownership of the old memory buffer.
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

inline uint32 StringTo32BitFnvHash(const FString& StringUnreal)
{
	TArray<TCHAR> Chars = StringUnreal.GetCharArray();

	if (Chars.Last() == '\0')
	{
		Chars.Pop();
	}

	uint32 Hash = 2166136261U;
	for (const auto& SingleChar : Chars)
	{
		Hash ^= SingleChar;
		Hash *= 16777619U;
	}

	return Hash;
}

//
// [GU]uid
//

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

//
// Enumerations, RigidBody.
//

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

//
// Enumerations, Constraint.
//

inline agx::Constraint2DOF::DOF Convert(EAGX_Constraint2DOFFreeDOF Dof)
{
	check(Dof == EAGX_Constraint2DOFFreeDOF::FIRST || Dof == EAGX_Constraint2DOFFreeDOF::SECOND);

	return Dof == EAGX_Constraint2DOFFreeDOF::FIRST ? agx::Constraint2DOF::FIRST
													: agx::Constraint2DOF::SECOND;
}

//
// Enumerations, Materials.
//

inline agx::FrictionModel::SolveType Convert(EAGX_ContactSolver ContactSolver)
{
	switch (ContactSolver)
	{
		case EAGX_ContactSolver::Direct:
			return agx::FrictionModel::SolveType::DIRECT;
		case EAGX_ContactSolver::Iterative:
			return agx::FrictionModel::SolveType::ITERATIVE;
		case EAGX_ContactSolver::Split:
			return agx::FrictionModel::SolveType::SPLIT;
		case EAGX_ContactSolver::DirectAndIterative:
			return agx::FrictionModel::SolveType::DIRECT_AND_ITERATIVE;
		case EAGX_ContactSolver::NotDefined:
			return agx::FrictionModel::SolveType::NOT_DEFINED;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "EAGX_ContactSolver literal with unknown value to "
					 "an agxModel::FrictionModel::SolveType literal."));
			return agx::FrictionModel::SolveType::NOT_DEFINED;
	}
}

inline EAGX_ContactSolver Convert(agx::FrictionModel::SolveType SolveType)
{
	switch (SolveType)
	{
		case agx::FrictionModel::SolveType::DIRECT:
			return EAGX_ContactSolver::Direct;
		case agx::FrictionModel::SolveType::ITERATIVE:
			return EAGX_ContactSolver::Iterative;
		case agx::FrictionModel::SolveType::SPLIT:
			return EAGX_ContactSolver::Split;
		case agx::FrictionModel::SolveType::DIRECT_AND_ITERATIVE:
			return EAGX_ContactSolver::DirectAndIterative;
		case agx::FrictionModel::SolveType::NOT_DEFINED:
			return EAGX_ContactSolver::NotDefined;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "EAGX_ContactSolver literal with unknown value to "
					 "an agxModel::FrictionModel::SolveType literal."));
			return EAGX_ContactSolver::NotDefined;
	}
}

inline agx::ContactMaterial::ContactReductionMode Convert(EAGX_ContactReductionMode Mode)
{
	switch (Mode)
	{
		case EAGX_ContactReductionMode::None:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_NONE;
		case EAGX_ContactReductionMode::Geometry:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_GEOMETRY;
		case EAGX_ContactReductionMode::All:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_ALL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an EAGX_ContactReductionMode literal "
					 "with unknown value to an agx::ContactMaterial::ContactReductionMode."))
			return agx::ContactMaterial::ContactReductionMode::REDUCE_NONE;
	}
}

inline EAGX_ContactReductionMode Convert(agx::ContactMaterial::ContactReductionMode Mode)
{
	switch (Mode)
	{
		case agx::ContactMaterial::REDUCE_NONE:
			return EAGX_ContactReductionMode::None;
		case agx::ContactMaterial::ContactReductionMode::REDUCE_GEOMETRY:
			return EAGX_ContactReductionMode::Geometry;
		case agx::ContactMaterial::ContactReductionMode::REDUCE_ALL:
			return EAGX_ContactReductionMode::All;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "agx::ContactMaterial::ContactReductionMode "
					 "with unknown value to an EAGX_ContactReductionMode."));
			return EAGX_ContactReductionMode::None;
	}
}

//
// Enumerations, Tire.
//

inline agxModel::TwoBodyTire::DeformationMode Convert(FTwoBodyTireBarrier::DeformationMode Mode)
{
	switch (Mode)
	{
		case FTwoBodyTireBarrier::RADIAL:
			return agxModel::TwoBodyTire::RADIAL;
		case FTwoBodyTireBarrier::LATERAL:
			return agxModel::TwoBodyTire::LATERAL;
		case FTwoBodyTireBarrier::BENDING:
			return agxModel::TwoBodyTire::BENDING;
		case FTwoBodyTireBarrier::TORSIONAL:
			return agxModel::TwoBodyTire::TORSIONAL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an FTwoBodyTireBarrier::DeformationMode "
					 "literal of unknown type to an agxModel::TwoBodyTire::DeformationMode "
					 "literal. Returning agxModel::TwoBodyTire::RADIAL."));
			return agxModel::TwoBodyTire::RADIAL;
	}
}

inline FTwoBodyTireBarrier::DeformationMode Convert(agxModel::TwoBodyTire::DeformationMode Mode)
{
	switch (Mode)
	{
		case agxModel::TwoBodyTire::RADIAL:
			return FTwoBodyTireBarrier::RADIAL;
		case agxModel::TwoBodyTire::LATERAL:
			return FTwoBodyTireBarrier::LATERAL;
		case agxModel::TwoBodyTire::BENDING:
			return FTwoBodyTireBarrier::BENDING;
		case agxModel::TwoBodyTire::TORSIONAL:
			return FTwoBodyTireBarrier::TORSIONAL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "agxModel::TwoBodyTire::DeformationMode "
					 "literal of unknown type to an FTwoBodyTireBarrier::DeformationMode "
					 "literal. Returning FTwoBodyTireBarrier::DeformationMode::RADIAL."));
			return FTwoBodyTireBarrier::DeformationMode::RADIAL;
	}
}

//
// Enumerations, Track.
//

inline EAGX_TrackWheelModel Convert(agxVehicle::TrackWheel::Model Model)
{
	switch (Model)
	{
		case agxVehicle::TrackWheel::IDLER:
			return EAGX_TrackWheelModel::Idler;
		case agxVehicle::TrackWheel::ROLLER:
			return EAGX_TrackWheelModel::Roller;
		case agxVehicle::TrackWheel::SPROCKET:
			return EAGX_TrackWheelModel::Sprocket;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown agxVehicle::TrackWheel::Model "
					 "literal to an EAGX_TrackWheelModel."));
			return EAGX_TrackWheelModel::Idler;
	}
}

inline agxVehicle::TrackWheel::Model Convert(EAGX_TrackWheelModel Model)
{
	switch (Model)
	{
		case EAGX_TrackWheelModel::Idler:
			return agxVehicle::TrackWheel::IDLER;
		case EAGX_TrackWheelModel::Roller:
			return agxVehicle::TrackWheel::ROLLER;
		case EAGX_TrackWheelModel::Sprocket:
			return agxVehicle::TrackWheel::SPROCKET;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown EAGX_TrackWheelModel "
					 "literal to an agxVehicle::TrackWheel::Model."));
			return agxVehicle::TrackWheel::IDLER;
	}
}

inline EAGX_MergedTrackNodeContactReduction Convert(
	agxVehicle::TrackInternalMergeProperties::ContactReduction Resolution)
{
	switch (Resolution)
	{
		case agxVehicle::TrackInternalMergeProperties::NONE:
			return EAGX_MergedTrackNodeContactReduction::None;
		case agxVehicle::TrackInternalMergeProperties::MINIMAL:
			return EAGX_MergedTrackNodeContactReduction::Minimal;
		case agxVehicle::TrackInternalMergeProperties::MODERATE:
			return EAGX_MergedTrackNodeContactReduction::Moderate;
		case agxVehicle::TrackInternalMergeProperties::AGGRESSIVE:
			return EAGX_MergedTrackNodeContactReduction::Aggressive;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown "
					 "agxVehicle::TrackInternalMergeProperties::ContactReduction "
					 "literal to an EAGX_MergedTrackNodeContactReduction."));
			return EAGX_MergedTrackNodeContactReduction::None;
	}
}

inline agxVehicle::TrackInternalMergeProperties::ContactReduction Convert(
	EAGX_MergedTrackNodeContactReduction Resolution)
{
	switch (Resolution)
	{
		case EAGX_MergedTrackNodeContactReduction::None:
			return agxVehicle::TrackInternalMergeProperties::NONE;
		case EAGX_MergedTrackNodeContactReduction::Minimal:
			return agxVehicle::TrackInternalMergeProperties::MINIMAL;
		case EAGX_MergedTrackNodeContactReduction::Moderate:
			return agxVehicle::TrackInternalMergeProperties::MODERATE;
		case EAGX_MergedTrackNodeContactReduction::Aggressive:
			return agxVehicle::TrackInternalMergeProperties::AGGRESSIVE;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown "
					 "EAGX_MergedTrackNodeContactReduction"
					 "literal to an agxVehicle::TrackInternalMergeProperties::ContactReduction."));
			return agxVehicle::TrackInternalMergeProperties::NONE;
	}
}

//
// Enumerations, Wire.
//

inline EWireNodeType Convert(agxWire::Node::Type Type)
{
	switch (Type)
	{
		case agxWire::Node::FREE:
			return EWireNodeType::Free;
		case agxWire::Node::EYE:
			return EWireNodeType::Eye;
		case agxWire::Node::BODY_FIXED:
			return EWireNodeType::BodyFixed;
		case agxWire::Node::CONNECTING:
			return EWireNodeType::Connecting;
		case agxWire::Node::STOP:
			return EWireNodeType::Stop;
		case agxWire::Node::CONTACT:
			return EWireNodeType::Contact;
		case agxWire::Node::SHAPE_CONTACT:
			return EWireNodeType::ShapeContact;
		case agxWire::Node::MISSING:
		case agxWire::Node::NOT_DEFINED:
			return EWireNodeType::Other;
	}

	UE_LOG(
		LogAGX, Warning, TEXT("Unknown AGX Dynamics wire node type %d. Defaulting to Other."),
		static_cast<int>(Type));
	return EWireNodeType::Other;
}

inline agxWire::Node::Type Convert(EWireNodeType Type)
{
	switch (Type)
	{
		case EWireNodeType::Free:
			return agxWire::Node::FREE;
		case EWireNodeType::Eye:
			return agxWire::Node::EYE;
		case EWireNodeType::BodyFixed:
			return agxWire::Node::BODY_FIXED;
		case EWireNodeType::Connecting:
			return agxWire::Node::CONNECTING;
		case EWireNodeType::Stop:
			return agxWire::Node::STOP;
		case EWireNodeType::Contact:
			return agxWire::Node::CONTACT;
		case EWireNodeType::ShapeContact:
			return agxWire::Node::SHAPE_CONTACT;
		case EWireNodeType::Other:
		case EWireNodeType::NUM_USER_CREATABLE:
			return agxWire::Node::NOT_DEFINED;
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT("Unknown Unreal Engine wire node type %d. Defaulting to NOT_DEFINED."),
		static_cast<int>(Type));
	return agxWire::Node::NOT_DEFINED;
}

inline EWireNodeNativeType ConvertNative(agxWire::Node::Type Type)
{
	// The values in EWireNodeNativeType must match those in agxWire::Node::Type.
	return static_cast<EWireNodeNativeType>(Type);
}

inline agxWire::Node::Type ConvertNative(EWireNodeNativeType Type)
{
	// The values in EWireNodeNativeType must match those in agxWire::Node::Type.
	return static_cast<agxWire::Node::Type>(Type);
}

//
// Enumerations, Logging.
//

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

inline ELogVerbosity::Type ConvertLogLevelVerbosity(agx::Notify::NotifyLevel Level)
{
	switch (Level)
	{
		case agx::Notify::NOTIFY_DEBUG:
			return ELogVerbosity::VeryVerbose;
		case agx::Notify::NOTIFY_INFO:
			return ELogVerbosity::Verbose;
		case agx::Notify::NOTIFY_WARNING:
			return ELogVerbosity::Warning;
		case agx::Notify::NOTIFY_ERROR:
			return ELogVerbosity::Error;

		// The following are not actual verbosity levels.
		case agx::Notify::NOTIFY_CLEAR:
		case agx::Notify::NOTIFY_END:
		case agx::Notify::NOTIFY_LOGONLY:
		case agx::Notify::NOTIFY_PUSH:
			return ELogVerbosity::VeryVerbose;
	}

	UE_LOG(
		LogAGX, Warning, TEXT("Unknown AGX Dynamics log verbosity %d. Defaulting to Warning."),
		static_cast<int>(Level));
	return ELogVerbosity::Warning;
}
