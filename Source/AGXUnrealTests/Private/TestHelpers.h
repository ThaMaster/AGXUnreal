
#pragma once

#include "Misc/AutomationTest.h"

namespace TestHelpers
{
	constexpr EAutomationTestFlags::Type DefaultTestFlags =
		static_cast<const EAutomationTestFlags::Type>(
			EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask);

	/// @todo Remove local TestEqual for FQuat once it's include in-engine.
	/// @see "Misc/AutomationTest.h"
	inline void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
		float Tolerance = KINDA_SMALL_NUMBER)
	{
		if (!Expected.Equals(Actual, Tolerance))
		{
			Test.AddError(FString::Printf(
				TEXT("Expected '%s' to be '%s', but it was %s within tolerance %f."), What,
				*Expected.ToString(), *Actual.ToString(), Tolerance));
		}
	}

}

#define BAIL_TEST_IF(expression)      \
	if (expression)                   \
	{                                 \
		TestFalse(#expression, true); \
		return;                       \
	}
