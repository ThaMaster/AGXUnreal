
#pragma once

// Unreal Engine includes.
#include "Engine/EngineTypes.h"
#include "Misc/AutomationTest.h"

class UWorld;

/**
 * A set of helper functions used by several Automation tests.
 */
namespace AgxAutomationCommon
{
	/**
	 * Get a pointer to the best guess for which world is the test world.
	 *
	 * There is some heuristics involved. Returns the world found in the first WorldContext that is
	 * either a play-in-editor context or a game context. For there to be such a world the tests
	 * must either be run from within the editor or from and command line with the parameter '-Game'
	 * passed to UE4Editor.
	 *
	 * I don't know how cooking/packaging/exporting the project affects this.
	 *
	 * @return The UWorld used for the Automation tests, or nullptr if no suitable world is found.
	 */
	UWorld* GetTestWorld();

	constexpr EAutomationTestFlags::Type DefaultTestFlags =
		static_cast<const EAutomationTestFlags::Type>(
			EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext |
			EAutomationTestFlags::ClientContext);

	/// @todo Remove local TestEqual for FQuat once it's include in-engine.
	/// @see "Misc/AutomationTest.h"
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
		float Tolerance = KINDA_SMALL_NUMBER);

	/// @todo Figure out how to use UEnum::GetValueAsString instead of this helper function.
	/// I get linker errors.
	FString WorldTypeToString(EWorldType::Type Type);

	// Not 'enum class' because I want implicit conversion to bool, with 'NoReason' being false.
	// We can do 'if (Reason) { <Cannot do world tests.> }'.
	enum NoWorldTestsReason
	{
		NoReason = 0, // It is safe to run world tests.
		TooManyWorlds,
		IllegalWorldType
	};

	FString GetNoWorldTestsReasonText(NoWorldTestsReason Reason);

	/**
	 * Perform the same checks as FLoadGameMapCommand in AutomationCommon, but return a reason on
	 * failure instead of crashing.
	 *
	 * @return The reason why world tests can't be run, or NoReason if world tests can be run.
	 */
	NoWorldTestsReason CanRunWorldTests();

	/**
	 * Get the file system path to an AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const TCHAR* ArchiveName);

	/**
	 * Get the file system path to an AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const FString& ArchiveName);

}

#define BAIL_TEST_IF(expression)      \
	if (expression)                   \
	{                                 \
		TestFalse(#expression, true); \
		return;                       \
	}

#define BAIL_TEST_IF_NO_WORLD()                                                                   \
	if (AgxAutomationCommon::NoWorldTestsReason Reason = AgxAutomationCommon::CanRunWorldTests()) \
	{                                                                                             \
		AddError(AgxAutomationCommon::GetNoWorldTestsReasonText(Reason));                         \
		return false;                                                                             \
	}
