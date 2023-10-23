// Copyright 2023, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_PlayInEditorUtils.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "AgxAutomationCommon.h"
#include "Containers/Map.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace AGX_PlayInEditorTest_helpers
{
	using namespace AGX_PlayInEditorUtils;

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

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
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

	const float SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
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

	static int32 NumTicks = 0;
	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
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

	const float SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
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
// Playground test starts here.
//
// The Playground level contains a mixture of several different things/mechanisms/scenarios and
// exists to make it easy to add play-in-editor test scenarios in the future. Simply add the
// scenario in the level and add the test logic for it here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FCheckPlaygroundStateCommand, float, SimTimeMax, ComponentMap, ComponentsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckPlaygroundStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	static int32 NumTicks = 0;
	if (ComponentsOfInterest.Num() == 0)
	{
		NumTicks = 0;
		auto FallDynamicBody =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Collisions", "FallDynamicBody");
		Test.TestNotNull("FallDynamicBody", FallDynamicBody);

		auto ConstrainedBody = GetComponentByName<UAGX_RigidBodyComponent>(
			TestWorld, "Collisions", "ConstrainedBody2");
		Test.TestNotNull("ConstrainedBody", ConstrainedBody);

		auto SlideBody =
			GetComponentByName<UAGX_RigidBodyComponent>(TestWorld, "Slide", "SlideBody");
		Test.TestNotNull("SlideBody", SlideBody);

		if (FallDynamicBody == nullptr || ConstrainedBody == nullptr || SlideBody == nullptr)
			return true;

		ComponentsOfInterest.Add("FallDynamicBody", FallDynamicBody);
		ComponentsOfInterest.Add("ConstrainedBody", ConstrainedBody);
		ComponentsOfInterest.Add("SlideBody", SlideBody);

		Test.TestTrue(
			"FallDynamicBody initial z pos", FallDynamicBody->GetComponentLocation().Z > 0.0);
		Test.TestTrue(
			"ConstrainedBody initial z pos", ConstrainedBody->GetComponentLocation().Z > 0.0);
		Test.TestTrue("SlideBody initial z pos", SlideBody->GetComponentLocation().Z > 0.0);
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(TestWorld);
	Test.TestNotNull("Simulation", Sim);
	if (Sim == nullptr)
		return true;

	const float SimTime = Sim->GetTimeStamp();
	{
		// Sanity check to avoid hanging forever if the Simulation is not ticking.
		NumTicks++;
		if (NumTicks > 1000 && FMath::IsNearlyZero(SimTime))
		{
			Test.AddError(FString::Printf(
				TEXT("SimTime too small: %f. The Simulation has not stepped as expected."),
				SimTime));
			return true;
		}
	}

	if (SimTime < SimTimeMax)
	{
		return false; // Continue ticking..
	}

	// At this point we have simulated to SimTimeMax. Check the final state.
	auto FallDynamicBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["FallDynamicBody"]);
	auto ConstrainedBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["ConstrainedBody"]);
	auto SlideBody = Cast<UAGX_RigidBodyComponent>(ComponentsOfInterest["SlideBody"]);
	Test.TestTrue(
		"FallDynamicBody final z pos", FallDynamicBody->GetComponentLocation().Z < -100.0);
	Test.TestTrue(
		"ConstrainedBody final z pos", ConstrainedBody->GetComponentLocation().Z < -100.0);
	Test.TestTrue("SlideBody final z pos", SlideBody->GetComponentLocation().Z < -100.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPlaygroundTest, "AGXUnreal.Game.AGX_PlayInEditorTest.Playground",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPlaygroundTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Playground/Playground");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	ComponentMap ComponentsOfInterest;
	float SimTimeMax = 5.0f;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckPlaygroundStateCommand(SimTimeMax, ComponentsOfInterest, *this));

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
	const TArray<FString> IgnoreLevels {"ComponentGallery"};

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
	AgxAutomationCommon::AddExpectedError(
		*this, TEXT("Could not allocate resource for Landscape Displacement Map for AGX Terrain "
					".*. There may be rendering issues."));
	using namespace AGX_PlayInEditorTest_helpers;
	const FString LevelPath = Parameters;
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(LevelPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));

	int TickCurrent = 0;
	int TickMax = 3;
	ADD_LATENT_AUTOMATION_COMMAND(FTickOnlyCommand(TickCurrent, TickMax));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);

	return true;
}

//
// Material Library test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckMaterialLibraryStateCommand, FAutomationTestBase&, Test);

bool FCheckMaterialLibraryStateCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;

	static int32 NumTicks = 0;
	NumTicks++;
	if (NumTicks > 1000)
	{
		Test.AddError("Level never began play even after many attempts.");
		return true;
	}

	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
	auto Box = GetComponentByName<UAGX_BoxShapeComponent>(TestWorld, "Actor", "BoxShape");
	Test.TestNotNull("Box", Box);

	auto Sphere = GetComponentByName<UAGX_SphereShapeComponent>(TestWorld, "Actor", "SphereShape");
	Test.TestNotNull("Sphere", Sphere);

	auto CMRegistrar = GetComponentByName<UAGX_ContactMaterialRegistrarComponent>(
		TestWorld, "Actor", "CMRegistrar");
	Test.TestNotNull("CMRegistrar", CMRegistrar);

	if (Box == nullptr || Sphere == nullptr || CMRegistrar == nullptr)
		return true;

	// Sanity check: ensure the materials used from the Material Library are still assigned.
	Test.TestTrue("Aluminium Library Shape Material not null", Box->ShapeMaterial != nullptr);
	Test.TestTrue("Steel Library Shape Material not null", Sphere->ShapeMaterial != nullptr);
	Test.TestTrue(
		"Steel-Aluminium Library Contact Material not null and assigned material pair",
		CMRegistrar->ContactMaterials.Num() == 1 && CMRegistrar->ContactMaterials[0] != nullptr &&
			CMRegistrar->ContactMaterials[0]->Material1 != nullptr &&
			CMRegistrar->ContactMaterials[0]->Material2 != nullptr);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMaterialLibraryTest, "AGXUnreal.Game.AGX_PlayInEditorTest.MaterialLibrary",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMaterialLibraryTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorTest_helpers;
	FString MapPath = FString("/Game/Tests/Test_MaterialLibrary");

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckMaterialLibraryStateCommand(*this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	return true;
}
