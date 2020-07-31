
#pragma once

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Misc/AutomationTest.h"

/// @todo Move most of the function definitions to TestHelper.cpp
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

	/// @todo Figure out how to use UEnum::GetVAlueAsString. I get linker errors.
	inline FString GetWorldTypeAsString(EWorldType::Type Type)
	{
		switch (Type)
		{
			case EWorldType::None:
				return TEXT("None");
			case EWorldType::Game:
				return TEXT("Game");
			case EWorldType::Editor:
				return TEXT("Editor");
			case EWorldType::PIE:
				return TEXT("PIE");
			case EWorldType::EditorPreview:
				return TEXT("EditorPreview");
			case EWorldType::GamePreview:
				return TEXT("GamePreview");
			case EWorldType::GameRPC:
				return TEXT("GameRPC");
			case EWorldType::Inactive:
				return TEXT("Inactive");
		}
	}

	// Not 'enum class' because I want implicit conversion to bool, with 'NoReason' being false.
	// We can do 'if (Reason) { /* Cannot do world tests. */ }'.
	enum NoWorldTestsReason
	{
		NoReason = 0, // It is safe to run world tests.
		TooManyWorlds,
		IllegalWorldType
	};

	inline FString GetNoWorldTestsReasonText(NoWorldTestsReason Reason)
	{
		switch (Reason)
		{
			case TooManyWorlds:
				return "Cannot run tests that need a world because there are too many world "
					   "contexts.";
			case IllegalWorldType:
				return FString::Printf(
					TEXT("Cannot run tests that need a world because the available world isn't a "
						 "'Game' world, it's a '%s' world."),
					*TestHelpers::GetWorldTypeAsString(
						GEngine->GetWorldContexts()[0].WorldType.GetValue()));
		}
		return FString();
	}

	/**
	 * Perform the same checks as FLoadGameMapCommand in AutomationCommon, but return false instead
	 * of crash on failure.
	 *
	 * @return The reason why world tests can't be run, or NoReason if world tests can be run.
	 */
	inline NoWorldTestsReason CanRunWorldTests()
	{
		const TIndirectArray<FWorldContext>& Worlds = GEngine->GetWorldContexts();
		if (Worlds.Num() != 1)
		{
			return NoWorldTestsReason::TooManyWorlds;
		}
		if (GEngine->GetWorldContexts()[0].WorldType != EWorldType::Game)
		{
			return NoWorldTestsReason::IllegalWorldType;
		}
		return NoWorldTestsReason::NoReason;
	}

}

#define BAIL_TEST_IF(expression)      \
	if (expression)                   \
	{                                 \
		TestFalse(#expression, true); \
		return;                       \
	}

#define BAIL_TEST_IF_NO_WORLD()                                                   \
	if (TestHelpers::NoWorldTestsReason Reason = TestHelpers::CanRunWorldTests()) \
	{                                                                             \
		AddError(TestHelpers::GetNoWorldTestsReasonText(Reason));                 \
		return false;                                                             \
	}
