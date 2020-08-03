
#pragma once

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Misc/AutomationTest.h"

/// @todo Move most of the function definitions to TestHelper.cpp
namespace TestHelpers
{
	// Copy of the hidden method GetAnyGameWorld() in AutomationCommon.cpp.
	// Marked as temporary there, hence, this one is temporary, too.
	//
	/// \TODO GetTestWorld doesn't work, I have only ever seen it return nullptr.
	/// What does it do, actually? When is it supposed to be used? Why does it work in
	/// AutomationCommon.cpp?
	///
	/// Answer:
	/// It does work, but the world must be enabled/running/ticking first. The way to enable the
	/// world when running unit tests from the command line through Unreal Editor is to pass `-Game`
	/// on the command line to UE4Editor. When I do that I get the same UWorld pointer from both
	/// GetTestWorld and FAGX_EditorUtilities::GetCurrentWorld.
	inline UWorld* GetTestWorld()
	{
		if (GEngine == nullptr)
		{
			UE_LOG(LogAGX, Warning, TEXT("Cannot get the test world because GEngine is nullptr."));
			return nullptr;
		}
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		if (WorldContexts.Num() == 0)
		{
			UE_LOG(LogAGX, Warning, TEXT("GEngine->GetWorldContexts() is empty."));
			return nullptr;
		}
		for (const FWorldContext& Context : WorldContexts)
		{
			bool bIsPieOrGame =
				Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game;
			if (bIsPieOrGame && Context.World() != nullptr)
			{
				return Context.World();
			}
		}
		UE_LOG(
			LogAGX, Warning, TEXT("Non of the %d WorldContexts contain a PIE or Game world."),
			WorldContexts.Num());
		return nullptr;
	}

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
