
#pragma once

#include "Misc/AutomationTest.h"

namespace TestHelpers
{
	constexpr EAutomationTestFlags::Type DefaultTestFlags =
		static_cast<const EAutomationTestFlags::Type>(
			EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask);

}

#define BAIL_TEST_IF(expression)      \
	if (expression)                   \
	{                                 \
		TestFalse(#expression, true); \
		return;                       \
	}
