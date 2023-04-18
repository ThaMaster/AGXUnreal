// Copyright 2023, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Terrain/AGX_Terrain.h"

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

	template <typename T>
	T* GetComponentByName(UWorld* TestWorld, const FString& ActorName, const FString& ComponentName)
	{
		TMap<FString, AActor*> Actors = GetActorsByName(TestWorld, {ActorName});
		AActor* Actor = Actors.FindRef(ActorName);
		if (Actor == nullptr)
			return nullptr;

		for (const auto& Component : Actor->GetComponents())
		{
			if (Component->GetName().Equals(ComponentName))
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
// Terrain Paging test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckTerrainPagingStateCommand, float, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckTerrainPagingStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		auto Chassi = GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Car", "Chassi");
		Test.TestNotNull("Chassi", Chassi);

		auto Untracked =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Untracked", "Untracked");
		Test.TestNotNull("Untracked", Untracked);

		if (Chassi == nullptr || Untracked == nullptr)
			return true;

		ComponentsOfInterest.Add("Chassi", Chassi);
		ComponentsOfInterest.Add("Untracked", Untracked);
		Test.TestTrue("Chassi body initial y pos", Chassi->GetComponentLocation().Y < 10.0);
		Test.TestTrue("Untracked body initial z pos", Chassi->GetComponentLocation().Z > 0.0);

		// Ensure ShapeMaterial is still assigned (sanity check that may fail due to backwards
		// compatibility breaks).
		auto ChassiBox = GetComponentByName<UAGX_BoxShapeComponent>(TestWorld, "Car", "ChassiBox");
		Test.TestNotNull("ChassiBox", ChassiBox);
		if (ChassiBox == nullptr)
			return true;

		Test.TestNotNull("ShapeMaterial of ChassiBox", ChassiBox->ShapeMaterial);
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
	auto ChassiBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["Chassi"]);
	auto UntrackedBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["Untracked"]);

	// The "Car" is tracked by the Terrain Pager and should have moved forwards.
	Test.TestTrue("Chassi body final y pos", ChassiBody->GetComponentLocation().Y > 50.0);

	// We expect the untracked body to fall through the landscape towards negative infinity since no
	// Terrain Tile has been created for it.
	Test.TestTrue("Untracked body final z pos", UntrackedBody->GetComponentLocation().Z < 0.0);

	// Ensure we have spawned some particles (there is a shovel in the Level).
	ActorMap TerrainActors = GetActorsByName(TestWorld, {"AGX_Terrain_1"});
	AAGX_Terrain* TerrainActor = Cast<AAGX_Terrain>(TerrainActors.FindRef("AGX_Terrain_1"));
	Test.TestNotNull("Terrain actor", TerrainActor);
	if (TerrainActor == nullptr)
		return true;

	Test.TestTrue("Has spawned particles", TerrainActor->GetNumParticles() > 5);

	// Ensure TerrainMaterial is assigned (sanity check that may fail due to backwards
	// compatibility breaks).
	Test.TestNotNull("Terrain Material", TerrainActor->TerrainMaterial);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTerrainPagingTest, "AGXUnreal.Game.AGX_PlayInEditorTest.TerrainPaging",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTerrainPagingTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_TerrainPaging");

	// AGX Dynamics gives us this error due to float to double conversion that we do when creating
	// the Native Shovel. The tolerance in AGX Dynamics is too small for our conversion to pass that
	// check. Therefore we add it as an expected error here as a work-around to be able to do this
	// test.
	AddExpectedError(
		TEXT("Shovel cutting direction is not normalized!"),
		EAutomationExpectedErrorFlags::Contains, 0);
	AddError(TEXT("Shovel cutting direction is not normalized!"));

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	float SimTimeMax = 4.f;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckTerrainPagingStateCommand(SimTimeMax, ComponentsOfInterest, *this));

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
