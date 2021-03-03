#include "Contacts/AGX_ContactPoint.h"

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
