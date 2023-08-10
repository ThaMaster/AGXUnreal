// Copyright 2023, Algoryx Simulation AB.

/*
 * This file contains unit tests for Rigid Body Component.
 */

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PlayInEditorUtils.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "AgxAutomationCommon.h"
#include "Editor.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

///
/// Rigid Body Velocity Damping test starts here.
///

// State owned by the test and carried between latent command invocations.
struct FAngularVelocityState
{
	AActor* Actor {nullptr};
	UAGX_RigidBodyComponent* UndampedBody {nullptr};
	UAGX_RigidBodyComponent* DampedBody {nullptr};
	double EndTimeStamp {-1.0};
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FBuildAngularVelocityCommand, TSharedPtr<FAngularVelocityState>, State);

bool FBuildAngularVelocityCommand::Update()
{
	check(State != nullptr);
	check(State->Actor == nullptr);
	check(GEditor != nullptr);
	check(GEditor->GetPIEWorldContext() != nullptr);
	check(GEditor->GetPIEWorldContext()->World() != nullptr);

	UWorld* World = GEditor->GetPIEWorldContext()->World();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);

	// Configure the simulation.
	Simulation->SetUniformGravity(FVector(ForceInitToZero));
	State->EndTimeStamp = Simulation->GetTimeStamp() + 1.0;

	// Spawn the Actor.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = FName("Angular Velocity Damping Actor");
	State->Actor = World->SpawnActor<AActor>(SpawnParameters);
	USceneComponent* RootComponent = NewObject<USceneComponent>(
		State->Actor, USceneComponent::GetDefaultSceneRootVariableName());
	State->Actor->SetRootComponent(RootComponent);
	State->Actor->AddInstanceComponent(RootComponent);
	RootComponent->RegisterComponent();

	// Created damped body.
	State->DampedBody = NewObject<UAGX_RigidBodyComponent>(State->Actor, TEXT("Damped Body"));
	State->DampedBody->Mobility = EComponentMobility::Movable;
	State->DampedBody->SetVelocity(FVector(1.0, 1.0, 1.0));
	State->DampedBody->SetAngularVelocity(FVector(1.0, 1.0, 1.0));
	State->DampedBody->SetLinearVelocityDamping(FVector(1.0, 2.0, 3.0));
	State->DampedBody->SetAngularVelocityDamping(FVector(1.0, 2.0, 3.0));
	State->Actor->AddInstanceComponent(State->DampedBody);
	State->DampedBody->RegisterComponent();

	// Create the undamped body.
	State->UndampedBody = NewObject<UAGX_RigidBodyComponent>(State->Actor, TEXT("Undamped Body"));
	State->UndampedBody->Mobility = EComponentMobility::Movable;
	State->UndampedBody->SetVelocity(FVector(1.0, 1.0, 1.0));
	State->UndampedBody->SetAngularVelocity(FVector(1.0, 1.0, 1.0));
	State->Actor->AddInstanceComponent(State->UndampedBody);
	State->UndampedBody->RegisterComponent();

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckAngularVelocityCommand, TSharedPtr<FAngularVelocityState>, State, FAutomationTestBase&,
	Test);

bool FCheckAngularVelocityCommand::Update()
{
	const UAGX_RigidBodyComponent* UndampedBody = State->UndampedBody;
	const FVector UndampedVel = UndampedBody->GetVelocity();
	const FVector UndampedAngVel = UndampedBody->GetAngularVelocity();
	Test.TestEqual(TEXT("Undamped Velocity"), UndampedVel, FVector(1.0, 1.0, 1.0));
	Test.TestEqual(TEXT("Undamped Angular Velocity"), UndampedAngVel, FVector(1.0, 1.0, 1.0));
	UE_LOG(
		LogAGX, Warning, TEXT("FCheckAngularVelocityCommand: Undamped velocity=%s."),
		*UndampedVel.ToString());
	UE_LOG(
		LogAGX, Warning, TEXT("FCheckAngularVelocityCommand: Undamped angular velocity=%s."),
		*UndampedAngVel.ToString());

	const UAGX_RigidBodyComponent* DampedBody = State->DampedBody;
	const FVector DampedVel = DampedBody->GetVelocity();
	const FVector DampedAngVel = DampedBody->GetAngularVelocity();
	AgxAutomationCommon::TestAllLess(
		Test, TEXT("Actual"), DampedVel, TEXT("Initial"), FVector(1.0, 1.0, 1.0));
	AgxAutomationCommon::TestAllLess(
		Test, TEXT("Actual"), DampedAngVel, TEXT("Initial"), FVector(1.0, 1.0, 1.0));
	AgxAutomationCommon::TestLess(Test, TEXT("Vel.Z"), DampedVel.Z, TEXT("Vel.Y"), DampedVel.Y);
	AgxAutomationCommon::TestLess(Test, TEXT("Vel.Y"), DampedVel.Y, TEXT("Vel.X"), DampedVel.X);
	AgxAutomationCommon::TestLess(Test, TEXT("AnglVel.Z"), DampedAngVel.Z, TEXT("AnglVel.Y"), DampedAngVel.Y);
	AgxAutomationCommon::TestLess(Test, TEXT("AnglVel.Y"), DampedAngVel.Y, TEXT("AnglVel.X"), DampedAngVel.X);
	UE_LOG(
		LogAGX, Warning, TEXT("FCheckAngularVelocityCommand: Damped velocity=%s."),
		*DampedVel.ToString());

	UE_LOG(
		LogAGX, Warning, TEXT("FCheckAngularVelocityCommand: Damped angular velocity=%s."),
		*DampedAngVel.ToString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAngularVelocityTest, "AGXUnreal.Game.AGX_RigidBody.VelocityDamping",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAngularVelocityTest::RunTest(const FString& Parameters)
{
	using namespace AGX_PlayInEditorUtils;

	// Must allocate the state on the free store since the latent commands will execute after
	// this function has returned and its local variables destroyed.
	TSharedPtr<FAngularVelocityState> State = MakeShared<FAngularVelocityState>();

	// Setup initial state.
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(EmptyMapPath))
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitUntilPIEUpCommand);
	ADD_LATENT_AUTOMATION_COMMAND(FBuildAngularVelocityCommand(State))
	ADD_LATENT_AUTOMATION_COMMAND(FTickUntilDynamicTimeStamp(&State->EndTimeStamp));

	// Run the checks.
	ADD_LATENT_AUTOMATION_COMMAND(FCheckAngularVelocityCommand(State, *this));

	// Restore clean state.
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(EmptyMapPath));

	return true;
}
