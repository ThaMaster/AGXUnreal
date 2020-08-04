#include "AgxAutomationCommon.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"

UWorld* AgxAutomationCommon::GetTestWorld()
{
	// Based on GetAnyGameWorld() in AutomationCommon.cpp.
	// That implementation has the following commend:
	//
	// @todo this is a temporary solution. Once we know how to get test's hands on a proper world
	// this function should be redone/removed

	if (GEngine == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot get the test world because GEngine is nullptr."));
		return nullptr;
	}
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	if (WorldContexts.Num() == 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot get the test world because GEngine->GetWorldContexts() is empty."));
		return nullptr;
	}
	for (const FWorldContext& Context : WorldContexts)
	{
		// It's not clear to me which worlds are OK to use when testing. Here, taken from
		// GetAnyGameWorld in the engine's AutomationCommon.cpp, we allow with PIE and Game.
		// However, in FLoadGameMapCommand, in the same AutomationCommon.cpp, only Game worlds, not
		// PIE, are accepted. So some things are allowed on both PIE and Game worlds, but map
		// loading is only allowed on Game worlds? Is there a way to create a hidden background
		// world used for the test only, detaching the entire test from the state of the rest of the
		// worlds?
		bool bIsPieOrGame =
			Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game;
		if (bIsPieOrGame && Context.World() != nullptr)
		{
			return Context.World();
		}
	}
	UE_LOG(
		LogAGX, Warning, TEXT("None of the %d WorldContexts contain a PIE or Game world."),
		WorldContexts.Num());
	return nullptr;
}

void AgxAutomationCommon::TestEqual(
	FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
	float Tolerance)
{
	if (!Expected.Equals(Actual, Tolerance))
	{
		Test.AddError(FString::Printf(
			TEXT("Expected '%s' to be '%s', but it was %s within tolerance %f."), What,
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
