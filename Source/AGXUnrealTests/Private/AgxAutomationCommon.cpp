#include "AgxAutomationCommon.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

UWorld* AgxAutomationCommon::GetTestWorld()
{
	// Based on GetAnyGameWorld() in AutomationCommon.cpp.
	// That implementation has the following comment:
	//
	// @todo this is a temporary solution. Once we know how to get test's hands on a proper world
	// this function should be redone/removed
	//
	// Keep an eye at the engine implementation and replace this once they provide a better way to
	// get the test world.

	if (GEngine == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot get the test world because GEngine is nullptr."));
		return nullptr;
	}
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	if (WorldContexts.Num() == 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get the test world because GEngine->GetWorldContexts() is empty."));
		return nullptr;
	}
	TArray<UWorld*> Candidates;
	for (const FWorldContext& Context : WorldContexts)
	{
		// It's not clear to me which worlds are OK to use when testing. Here, taken from
		// GetAnyGameWorld in the engine's AutomationCommon.cpp, we allow with PIE and Game.
		// However, in FLoadGameMapCommand, in the same AutomationCommon.cpp, only Game worlds, not
		// PIE, are accepted. So some things are allowed on both PIE and Game worlds, but map
		// loading is only allowed on Game worlds? Is there a way to create a hidden background
		// world used for the test only, detaching the entire test from the state of the rest of the
		// worlds?
		EWorldType::Type Type = Context.WorldType;
		bool bIsPieOrGame = Type == EWorldType::PIE || Type == EWorldType::Game;
		if (bIsPieOrGame && Context.World() != nullptr)
		{
			Candidates.Add(Context.World());
		}
	}
	if (Candidates.Num() == 0)
	{
		UE_LOG(
			LogAGX, Error, TEXT("None of the %d WorldContexts contain a PIE or Game world."),
			WorldContexts.Num());
		return nullptr;
	}
	else if (Candidates.Num() == 1)
	{
		return Candidates[0];
	}
	else if (Candidates.Num() > 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Got more than one world that looks like a test world. Don't know which one "
				 "should be used."))
		return nullptr;
	}
	checkNoEntry();
	return nullptr;
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FRotator& Actual, const FRotator& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s' but it was '%s', with tolerance %f."), What,
			*Expected.ToString(), *Actual.ToString(), Tolerance));
	}
}

FString AgxAutomationCommon::WorldTypeToString(EWorldType::Type Type)
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

FString AgxAutomationCommon::GetNoWorldTestsReasonText(NoWorldTestsReason Reason)
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
				*AgxAutomationCommon::WorldTypeToString(
					GEngine->GetWorldContexts()[0].WorldType.GetValue()));
	}
	return FString();
}

AgxAutomationCommon::NoWorldTestsReason AgxAutomationCommon::CanRunWorldTests()
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

FString AgxAutomationCommon::GetArchivePath(const TCHAR* ArchiveName)
{
	FString ProjectDir = FPaths::ProjectDir();
	FPaths::CollapseRelativeDirectories(ProjectDir);
	ProjectDir = FPaths::ConvertRelativePathToFull(ProjectDir);
	const FString ArchivesDir = ProjectDir.Replace(
		TEXT("/AGXUnrealDev/"), TEXT("/AGX_Dynamics_scenes/"), ESearchCase::CaseSensitive);
	const FString ArchivePath = FPaths::Combine(ArchivesDir, ArchiveName);
	if (FPaths::FileExists(ArchivePath))
	{
		return ArchivePath;
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Did not find full path for AGX Dynamics archive '%s'. Searched in '%s'."),
			ArchiveName, *ArchivesDir)
		return FString();
	}
}

FString AgxAutomationCommon::GetArchivePath(const FString& ArchiveName)
{
	return GetArchivePath(*ArchiveName);
}

bool AgxAutomationCommon::FLogWarningAgxCommand::Update()
{
	UE_LOG(LogAGX, Warning, TEXT("%s"), *Message);
	return true;
}

bool AgxAutomationCommon::FLogErrorAgxCommand::Update()
{
	UE_LOG(LogAGX, Error, TEXT("%s"), *Message);
	return true;
}

bool AgxAutomationCommon::FWaitNTicks::Update()
{
	--NumTicks;
	return NumTicks <= 0;
}

bool AgxAutomationCommon::FCheckWorldsCommand::Update()
{
	UWorld* TestWorld = AgxAutomationCommon::GetTestWorld();
	UWorld* CurrentWorld = FAGX_EditorUtilities::GetCurrentWorld();
	UE_LOG(LogAGX, Warning, TEXT("TestWorld:    %p"), (void*) TestWorld);
	UE_LOG(LogAGX, Warning, TEXT("CurrentWorld: %p"), (void*) CurrentWorld);
	Test.TestEqual(TEXT("Worlds"), TestWorld, CurrentWorld);
	Test.TestNotNull("TestWorld", TestWorld);
	Test.TestNotNull("CurrentWorld", CurrentWorld);
	return true;
}

bool AgxAutomationCommon::FTickUntilCommand::Update()
{
	return World->GetTimeSeconds() >= Time;
}

AgxAutomationCommon::FWaitWorldDuration::FWaitWorldDuration(UWorld*& InWorld, float InDuration)
	: World(InWorld)
	, Duration(InDuration)
{
}

bool AgxAutomationCommon::FWaitWorldDuration::Update()
{
	if (EndTime < 0.0f)
	{
		EndTime = World->GetTimeSeconds() + Duration;
	}
	return World->GetTimeSeconds() >= EndTime;
}

AgxAutomationCommon::FCheckWorldsTest::FCheckWorldsTest()
	: FAutomationTestBase(TEXT("FCheckWorldsTest"), false)
{
}

uint32 AgxAutomationCommon::FCheckWorldsTest::GetTestFlags() const
{
	return DefaultTestFlags;
}

uint32 AgxAutomationCommon::FCheckWorldsTest::GetRequiredDeviceNum() const
{
	return 1;
}

FString AgxAutomationCommon::FCheckWorldsTest::GetBeautifiedTestName() const
{
	return TEXT("AGXUnreal.CheckWorlds");
}

void AgxAutomationCommon::FCheckWorldsTest::GetTests(
	TArray<FString>& OutBeutifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeutifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(FString());
}

bool AgxAutomationCommon::FCheckWorldsTest::RunTest(const FString& InParameter)
{
	UE_LOG(
		LogAGX, Warning, TEXT("Running test '%s' with parameter '%s'."), *GetTestName(),
		*InParameter);

	ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FCheckWorldsCommand(*this));
	return true;
}

// We must create an instantiate of the test class for the testing framework to find it.
namespace
{
	AgxAutomationCommon::FCheckWorldsTest CheckWorldsTest;
}

AgxAutomationCommon::FAgxAutomationTest::FAgxAutomationTest(
	const FString& InClassName, const FString& InBeautifiedName)
	: FAutomationTestBase(InClassName, false)
	, BeautifiedTestName(InBeautifiedName)
{
}

uint32 AgxAutomationCommon::FAgxAutomationTest::GetTestFlags() const
{
	return DefaultTestFlags;
}

uint32 AgxAutomationCommon::FAgxAutomationTest::GetRequiredDeviceNum() const
{
	return 1;
}

FString AgxAutomationCommon::FAgxAutomationTest::GetBeautifiedTestName() const
{
	return BeautifiedTestName;
}

void AgxAutomationCommon::FAgxAutomationTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(FString());
};
