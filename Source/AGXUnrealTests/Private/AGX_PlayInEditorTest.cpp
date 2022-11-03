// Copyright 2022, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"

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
}

// For some reason, the DEFINE_LATENT_AUTOMATION_COMMAND macro fails when using TMap<FString,
// AActor*> directly.
using ActorMap = TMap<FString, AActor*>;


//
// FallingBox test starts here.
//

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FCheckFallinBoxMovedCommand, int, TickCurrent, int, TickMax, ActorMap, ActorsOfInterest,
	FAutomationTestBase&, Test);

bool FCheckFallinBoxMovedCommand::Update()
{
	using namespace AGX_PlayInEditorTest_helpers;
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	if (TickCurrent == 0)
	{
		UWorld* TestWorld = GEditor->GetPIEWorldContext()->World();
		ActorsOfInterest = GetActorsByName(TestWorld, {"BoxActor"});

		Test.TestTrue("Found actor of interest", ActorsOfInterest.Contains("BoxActor"));
		if (!ActorsOfInterest.Contains("BoxActor"))
		{
			return true;
		}

		auto Body =
			GetComponentByName<UAGX_RigidBodyComponent>(*ActorsOfInterest["BoxActor"], "BoxBody");
		Test.TestNotNull("BoxBody", Body);
		if (Body == nullptr)
			return true;
		Test.TestTrue("Body initial z pos", Body->GetComponentLocation().Z > 299.0);
	}

	TickCurrent++;
	if (TickCurrent < TickMax)
		return false;

	// At this point we have ticked to TickMax.
	auto Body =
		GetComponentByName<UAGX_RigidBodyComponent>(*ActorsOfInterest["BoxActor"], "BoxBody");
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

	ActorMap ActorsOfInterest;
	int TickCurrent = 0;
	int TickMax = 5;
	ADD_LATENT_AUTOMATION_COMMAND(
		FCheckFallinBoxMovedCommand(TickCurrent, TickMax, ActorsOfInterest, *this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);

	return true;
}

//
// PlayingAllExampleLevels starts here.
//