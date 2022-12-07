// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_PlayInEditorTest_helpers
{
	TMap<FString, AActor*> GetActorsByName(UWorld* TestWorld, const TArray<FString>& Names)
	{
		TMap<FString, AActor*> FoundActors;
		if (TestWorld == nullptr)
		{
			return FoundActors;
		}

		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(TestWorld, AActor::StaticClass(), AllActors);
		for (const FString& Name : Names)
		{
			AActor** FoundActor = AllActors.FindByPredicate(
				[&Name](AActor* Actor) { return Actor->GetActorLabel().Equals(Name); });
			if (FoundActor != nullptr)
			{
				FoundActors.Add(Name, *FoundActor);
			}
		}

		return FoundActors;
	}

	template <typename T>
	T* GetComponentByName(const AActor& Owner, const FString& Name)
	{
		for (const auto& Component : Owner.GetComponents())
		{
			if (Component->GetName().Equals(Name))
			{
				return Cast<T>(Component);
			}
		}

		return nullptr;
	}

	DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
		FTickOnlyCommand, int, TickCurrent, int, TickMax, FAutomationTestBase&, Test);

	bool FTickOnlyCommand::Update()
	{
		using namespace AGX_PlayInEditorTest_helpers;
		if (!GEditor->IsPlayingSessionInEditor())
		{
			return false;
		}

		TickCurrent++;
		if (TickCurrent < TickMax)
			return false;

		return true;
	}
}

// For some reason, the DEFINE_LATENT_AUTOMATION_COMMAND macro fails when using TMap<FString,
// AActor*> directly.
using ActorMap = TMap<FString, AActor*>;
using ComponentMap = TMap<FString, UActorComponent*>;

//
// FallingBox test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckFallinBoxMovedCommand, float, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckFallinBoxMovedCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		ActorMap BoxActors = GetActorsByName(TestWorld, {"BoxActor"});

		Test.TestTrue("Found actor of interest", BoxActors.Contains("BoxActor"));
		if (!BoxActors.Contains("BoxActor"))
		{
			return true;
		}

		auto Body = GetComponentByName<UAGX_RigidBodyComponent>(*BoxActors["BoxActor"], "BoxBody");
		Test.TestNotNull("BoxBody", Body);
		if (Body == nullptr)
			return true;

		ComponentsOfInterest.Add("BoxBody", Body);
		Test.TestTrue("Body initial z pos", Body->GetComponentLocation().Z > 299.0);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	if (Sim->GetTimeStamp() < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have ticked to TickMax.
	auto Body = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["BoxBody"]);
	Test.TestTrue("Body final z pos", Body->GetComponentLocation().Z < 299.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFallingBoxTest, "AGXUnreal.Game.AGX_PlayInEditorTest.FallingBox",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFallingBoxTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/FallingBox");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	float SimTimeMax = 0.5f;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckFallinBoxMovedCommand(SimTimeMax, ComponentsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}

//
// StepExampleLevels starts here.
//

/**
 * A complex automation test lets us define GetTests() from where we can append to
 * OutBeautifiedNames and OutTestCommands. For each entry in OutBeautifiedNames, a test with that
 * name will be run, where RunTest is called and the parameter passed to RunTest is the
 * corresponding element in OutTestCommands.
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FStepExampleLevelsTest, "AGXUnreal.Game.AGX_PlayInEditorTest.StepExampleLevels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Collect all levels inside Content/Levels.
void FStepExampleLevelsTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	// ComponentGallery ignored because it produces several errors on Play: for example Constraints
	// without a Body.
	// SimpleTerrain and AdvancedTerrain are ignored because they will attempt to resize the
	// Landscape Displacement map texture which is not allowed in this Unit test context apparently.
	const TArray<FString> IgnoreLevels {"ComponentGallery", "SimpleTerrain", "AdvancedTerrain"};

	const FString LevelsDir = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Levels"));
	TArray<FString> FoundAssetes;
	IFileManager::Get().FindFiles(FoundAssetes, *LevelsDir, TEXT("umap"));

	for (const FString& LevelFullPath : FoundAssetes)
	{
		const FString LevelName = FPaths::GetBaseFilename(LevelFullPath);
		if (IgnoreLevels.Contains(LevelName))
			continue;

		OutBeautifiedNames.Add(LevelName);
		OutTestCommands.Add(FString::Printf(TEXT("/Game/Levels/%s"), *LevelName));
	}
}

bool FStepExampleLevelsTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	const FString LevelPath = Parameters;
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(LevelPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	int TickCurrent = 0;
	int TickMax = 3;
	ADD_LATENT_AUTOMATION_COMMAND(FTickOnlyCommand(TickCurrent, TickMax, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);

	return true;
}
