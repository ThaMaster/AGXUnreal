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

namespace
{
	bool TestHasNative(const FAGX_ContactPoint& ContactPoint, const TCHAR* AttributeName)
	{
		if (ContactPoint.HasNative())
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get %s from a ContactPoint that doesn't have a native AGX Dynamics "
				 "representation"),
			AttributeName);
		return false;
	}
}

bool FAGX_ContactPoint::IsEnabled() const
{
	if (!TestHasNative(*this, TEXT("Enabled")))
	{
		return false;
	}
	return Barrier.IsEnabled();
}

float FAGX_ContactPoint::GetDepth() const
{
	if (!TestHasNative(*this, TEXT("Depth")))
	{
		return 0.0f;
	}
	return Barrier.GetDepth();
}

FVector FAGX_ContactPoint::GetLocation() const
{
	if (!TestHasNative(*this, TEXT("Location")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetLocation();
}

FVector FAGX_ContactPoint::GetNormal() const
{
	if (!TestHasNative(*this, TEXT("Normal")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetNormal();
}

FVector FAGX_ContactPoint::GetTangentU() const
{
	if (!TestHasNative(*this, TEXT("TangentU")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentU();
}

FVector FAGX_ContactPoint::GetTangentV() const
{
	if (!TestHasNative(*this, TEXT("TangentV")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentV();
}

FVector FAGX_ContactPoint::GetForce() const
{
	if (!TestHasNative(*this, TEXT("Force")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetForce();
}

FVector FAGX_ContactPoint::GetNormalForce() const
{
	if (!TestHasNative(*this, TEXT("NormalForce")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetNormalForce();
}

FVector FAGX_ContactPoint::GetTangentialForce() const
{
	if (!TestHasNative(*this, TEXT("TangentialForce")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetTangentialForce();
}

FVector FAGX_ContactPoint::GetLocalForce() const
{
	if (!TestHasNative(*this, TEXT("LocalForce")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetLocalForce();
}

FVector FAGX_ContactPoint::GetVelocity() const
{
	if (!TestHasNative(*this, TEXT("Velocity")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetVelocity();
}

FVector FAGX_ContactPoint::GetWitnessPoint(int32 Index) const
{
	if (!TestHasNative(*this, TEXT("WitnessPoint")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.GetWitnessPoint(Index);
}

float FAGX_ContactPoint::GetArea() const
{
	if (!TestHasNative(*this, TEXT("Area")))
	{
		return 0.0f;
	}
	return Barrier.GetArea();
}
