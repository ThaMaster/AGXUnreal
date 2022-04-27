// Copyright 2022, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Constraints/AGX_HingeConstraintComponent.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "GameFramework/Actor.h"

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckImportedRealIntervals, FAutomationTestBase&, Test);

bool FCheckImportedRealIntervals::Update()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	UE_LOG(
		LogAGX, Warning, TEXT("In check: World=0x%x"),
		(void*) FAGX_EditorUtilities::GetCurrentWorld());

	if (World == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("FCheckImportedRealIntervals: Could not get world."));
		return true;
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
	UE_LOG(LogAGX, Error, TEXT("FCheckImportedRealIntervals: Found %d Actors."), Actors.Num());
	AActor* HingeActor = nullptr;
	for (AActor* Actor : Actors)
	{
		UE_LOG(
			LogAGX, Error, TEXT("FCheckImportedRealIntervals:   - %s of type %s."),
			*Actor->GetName(), *Actor->GetClass()->GetName());

		if (Actor->GetName() == TEXT("HingeActor"))
		{
			if (HingeActor != nullptr)
			{
				UE_LOG(LogAGX, Error, TEXT("Found two Actors named 'HingeActor'. Something is wrong"));
			}
			HingeActor = Actor;
		}
	}

	if (HingeActor == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Did not find Hinge Actor."));
		return true;
	}

	TArray<UAGX_HingeConstraintComponent*> Hinges;
	HingeActor->GetComponents(Hinges);
	if (Hinges.Num() == 0)
	{
		UE_LOG(LogAGX, Error, TEXT("Did not find any Hinge in Actor '%s'."), *HingeActor->GetName());
		return true;
	}
	if (Hinges.Num() > 1)
	{
		UE_LOG(LogAGX, Error, TEXT("Found too many Hinges in Actor '%s'. Found %d."), *HingeActor->GetName(), Hinges.Num());
		return true;
	}

	UAGX_HingeConstraintComponent* Hinge = Hinges[0];
	FAGX_ConstraintRangePropertyPerDof& ForceRanges = Hinge->ForceRange;
	UE_LOG(LogAGX, Warning, TEXT("T1: (%f, %f)"), ForceRanges.Translational_1.Min, ForceRanges.Translational_1.Max);
	return true;
}

/**
 * Unit test that ensures that we can open levels saved before we switched force ranges from
 * FFloatInterval to FAGX_RealInterval.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRealIntervalBackwardsCompatibilityTest, "AGXUnreal.Editor.BackwardsCompatibility.RealInterval",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

bool FRealIntervalBackwardsCompatibilityTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogAGX, Warning, TEXT("\n\n\n\nRunning Real Interval Backwards Compatibility Test."));

	UE_LOG(
		LogAGX, Warning, TEXT("At start of test: World=0x%x"),
		(void*) FAGX_EditorUtilities::GetCurrentWorld());

	static const FString MapName {"/Game/Tests/BackwardsCompatibility/PreAGXRealInterval"};
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapName));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckImportedRealIntervals(*this));
	return true;
}

// WITH_DEV_AUTOMATION_TESTS
#endif
