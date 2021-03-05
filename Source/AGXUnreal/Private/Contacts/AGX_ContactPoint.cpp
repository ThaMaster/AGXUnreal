#include "Contacts/AGX_ContactPoint.h"

#include "AGX_LogCategory.h"

FAGX_ContactPoint::FAGX_ContactPoint(const FAGX_ContactPoint& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_ContactPoint::FAGX_ContactPoint(FContactPointBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

FAGX_ContactPoint& FAGX_ContactPoint::operator=(const FAGX_ContactPoint& Other)
{
	Barrier = Other.Barrier;
	return *this;
}

bool FAGX_ContactPoint::HasNative() const
{
	return Barrier.HasNative();
}

bool FAGX_ContactPoint::IsEnabled() const
{
	check(HasNative());
	return Barrier.IsEnabled();
}

float FAGX_ContactPoint::GetDepth() const
{
	check(HasNative());
	return Barrier.GetDepth();
}

FVector FAGX_ContactPoint::GetLocation() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get Point from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetLocation();
}

FVector FAGX_ContactPoint::GetNormal() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get Normal from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetNormal();
}

FVector FAGX_ContactPoint::GetTangentU() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get TangentU from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentU();
}

FVector FAGX_ContactPoint::GetTangentV() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get TangentV from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentV();
}

FVector FAGX_ContactPoint::GetForce() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get Force from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetForce();
}

FVector FAGX_ContactPoint::GetNormalForce() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get NormalForce from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetNormalForce();
}

FVector FAGX_ContactPoint::GetTangentialForce() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get TangentialForce from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentialForce();
}

FVector FAGX_ContactPoint::GetLocalForce() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get LocalForce from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetLocalForce();
}

FVector FAGX_ContactPoint::GetVelocity() const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get Velocity from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetVelocity();
}

FVector FAGX_ContactPoint::GetWitnessPoint(int32 Index) const
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get WitnessPoint from contact point without a native AGX Dynamics "
				 "representation."));
		return FVector::ZeroVector;
	}
	return Barrier.GetWitnessPoint(Index);
}

float FAGX_ContactPoint::GetArea() const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot get Area fron contact point without a native AGX Dynaics"
								   "representation."));
		return 0.0f;
	}
	return Barrier.GetArea();
}
