// Copyright 2022, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics require the AGX-Wires license for wire import. Our GitLab CI
// runtime environment currently doesn't have an AGX Dynamics license so the
// wire import test always fails. For now the test is disabled through this
// preprocessor flag. See internal issue 495. Remove the preprocessor guards
// once the GitLab CI runtime has an AGX Dynamics license.
#define AGX_TEST_WIRE_IMPORT 0

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToSingleActor.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Materials/AGX_ContactMaterialBase.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_MaterialBase.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#if AGX_TEST_WIRE_IMPORT
#include "Wire/AGX_WireComponent.h"
#endif

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

/*
 * This file contains a set of tests for AGX_ImporterToSingleActor, which imports an AGX
 * Dynamics archive into the current world as a single Actor that contains ActorComponents for each
 * imported object.
 *
 * Search for "test starts here." within this file to find the start of each test case.
 */

/**
 * Latent Command that imports an AGX Dynamics archive into a single actor. A pointer to the Actor
 * created to hold the imported objects is stored in the Contents parameter.
 * @param ArchiveName The AGX Dynamics archive to import.
 * @param Contents Pointer set to point to the Actor containing the imported objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FImportArchiveSingleActorCommand, FString, ArchiveName, AActor*&, Contents,
	FAutomationTestBase&, Test);

bool FImportArchiveSingleActorCommand::Update()
{
	if (ArchiveName.IsEmpty())
	{
		Test.AddError(TEXT("FImportArchiveSingleActorCommand not given an archive to import."));
		return true;
	}
	FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(ArchiveName);
	if (ArchiveFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find an archive named '%s'."), *ArchiveName));
		return true;
	}
	Contents = AGX_ImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
	Test.TestNotNull(TEXT("Contents"), Contents);
	return true;
}

/**
 * Latent Command that imports an URDF model into a single actor. A pointer to the Actor
 * created to hold the imported objects is stored in the Contents parameter.
 * @param FileName The URDF file to import.
 * @param Contents Pointer set to point to the Actor containing the imported objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
	FImportURDFSingleActorCommand, FString, FileName, FString, PackagePath, AActor*&, Contents,
	FAutomationTestBase&, Test);

bool FImportURDFSingleActorCommand::Update()
{
	if (FileName.IsEmpty())
	{
		Test.AddError(TEXT("FImportURDFSingleActorCommand not given a file to import."));
		return true;
	}
	FString UrdfFilePath = AgxAutomationCommon::GetTestScenePath(FileName);
	if (UrdfFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find an URDF file named '%s'."), *FileName));
		return true;
	}
	Contents = AGX_ImporterToSingleActor::ImportURDF(UrdfFilePath, PackagePath);
	Test.TestNotNull(TEXT("Contents"), Contents);
	return true;
}

//
// EmptyScene test starts here.
//

/**
 * Latent Command testing that the empty scene was imported correctly.
 * @param Contents The Actor that was created by the archive importer to hold the imported objects.
 * @param Test The Automation test that contains this Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckEmptySceneImportedCommand, AActor*&, Contents, FAutomationTestBase&, Test);

bool FCheckEmptySceneImportedCommand::Update()
{
	UWorld* World = AgxAutomationCommon::GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}

	// The Actor's only component should be the root component.
	TArray<UActorComponent*> Components;
	Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 1);
	USceneComponent* SceneRoot =
		AgxAutomationCommon::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);

	// The Actor should have been created in the test world.
	Test.TestEqual(TEXT("The actor should be in the test world."), Contents->GetWorld(), World);
	Test.TestTrue(TEXT("The actor should be in the test world."), World->ContainsActor(Contents));

	return true;
}

/**
 * Latent Command that removes everything that was created by the Import Empty Scene test. Actual
 * removal isn't done immediately by Unreal Engine so the first call to Update will return false
 * so that the removal is completed before the next Latent Command starts.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FClearEmptySceneImportedCommand, AActor*&, Contents);
bool FClearEmptySceneImportedCommand::Update()
{
	if (Contents == nullptr)
	{
		// The removal happened the previous tick so it's safe to return true and complete this
		// Latent Command now.
		return true;
	}

	UWorld* World = AgxAutomationCommon::GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}
	World->DestroyActor(Contents);
	Contents = nullptr;

	// Return false so the engine get a tick to do the actual removal.
	return false;
}

/**
 * Test that an empty AGX Dynamics archive can be imported, that the archive Actor root is created
 * as it should, and that it is added to the world.
 */
class FImporterToSingleActor_EmptySceneTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_EmptySceneTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_EmptySceneTest"),
			  TEXT("AGXUnreal.Game.ImporterToSingleActor.EmptyScene"))
	{
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		BAIL_TEST_IF_NO_WORLD(false)
		BAIL_TEST_IF_WORLDS_MISMATCH(false)

		/// @todo I would like to load a fresh map before doing the actual test, which it would seem
		/// one does with FLoadGameMapCommand and FWaitForMapToLoadCommand, but including them
		/// causes the world to stop ticking. Figure out why. For now I just hope that loading the
		/// map as a unit test launch command line parameter is good enough. Not sure how multiple
		/// import tests interact though. Some form of level cleanup Latent Command at the end of
		/// each test may be required. I really hope multiple tests don't run concurrently in the
		/// same world.
#if 0
		// ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")));
		// ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("empty_scene.agx", Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckEmptySceneImportedCommand(Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FClearEmptySceneImportedCommand(Contents));
		ADD_LATENT_AUTOMATION_COMMAND(AgxAutomationCommon::FWaitNTicks(1));

		return true;
	}

private:
	AActor* Contents = nullptr;
};

namespace
{
	FImporterToSingleActor_EmptySceneTest ImporterToSingleActor_EmptySceneTest;
}

//
// SingleSphere test starts here.
//

class FImporterToSingleActor_SingleSphereTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSingleSphereImportedCommand, FImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreInitialTimes, FImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreResultingTimes, FImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSphereHasMoved, FImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSingleSphereImportedCommand, AActor*&, Contents);

class FImporterToSingleActor_SingleSphereTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_SingleSphereTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_SingleSphereTest"),
			  TEXT("AGXUnreal.Game.ImporterToSingleActor.SingleSphere"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* SphereBody = nullptr;
	FVector StartPosition;
	FVector StartVelocity;
	float StartAgxTime = -1.0f;
	float StartUnrealTime = -1.0f;
	float EndUnrealTime = -1.0f;
	float EndAgxTime = -1.0f;
	int32 NumTicks = 0;

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace AgxAutomationCommon;
		BAIL_TEST_IF_CANT_SIMULATE(false)
		World = AgxAutomationCommon::GetTestWorld();
		if (World == nullptr)
		{
			AddError(TEXT("Do not have a test world, cannot test SingleSphere import."));
		}
		Simulation = UAGX_Simulation::GetFrom(World);
		if (Simulation == nullptr)
		{
			AddError(TEXT("Do not have a simulation, cannot test SingleSphere import."));
		}

		// See comment in FImporterToSingleActor_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("single_sphere_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSingleSphereImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FStoreInitialTimes(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitWorldDuration(World, 1.0f))
		ADD_LATENT_AUTOMATION_COMMAND(FStoreResultingTimes(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSphereHasMoved(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSingleSphereImportedCommand(Contents))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitNTicks(1))

		return true;
	}
};

namespace
{
	FImporterToSingleActor_SingleSphereTest ImporterToSingleActor_SingleSphereTest;
}

/*
 * Check that the expected state has been created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script single_sphere.agxPy.
 */
bool FCheckSingleSphereImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.World == nullptr || Test.Simulation == nullptr || Test.Contents == nullptr)
	{
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 3);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* SphereBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("SphereBody"));
	UAGX_SphereShapeComponent* SphereShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("SphereGeometry"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("SphereBody"), SphereBody);
	Test.TestNotNull(TEXT("SphereShape"), SphereShape);

	if (SphereBody == nullptr || SphereShape == nullptr)
	{
		Test.AddError("No sphere body found in the level, cannot continue.");
		return true;
	}

	// Name.
	{
		Test.TestEqual("Sphere name", SphereBody->GetFName(), FName(TEXT("SphereBody")));
	}

	// Position.
	{
		FVector Actual = SphereBody->GetComponentLocation();
		// The position, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.00000000000000000000e+01f, 2.00000000000000000000e+01f, 3.00000000000000000000e+01f);
		FVector Expected = AgxToUnrealVector(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere position"), Actual, Expected);
	}

	// Rotation.
	{
		FRotator Actual = SphereBody->GetComponentRotation();
		// The rotation, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.01770284974289526581e+00f, -2.65482457436691521302e-01f,
			-1.54866776461627897454e+00f);
		FRotator Expected = AgxToUnrealEulerAngles(ExpectedAgx);
		TestEqual(Test, TEXT("Sphere rotation"), Actual, Expected);
	}

	// Velocity.
	{
		FVector Actual = SphereBody->Velocity;
		// The velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.00000000000000000000e+00f, 2.00000000000000000000e+00f, 3.00000000000000000000e+00f);
		FVector Expected = AgxToUnrealVector(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere linear velocity"), Actual, Expected);
	}

	// Angular velocity.
	{
		FVector Actual = SphereBody->AngularVelocity;
		// The angular velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(
			1.10000000000000000000e+01f, 1.20000000000000000000e+01f, 1.30000000000000000000e+01f);
		FVector Expected = AgxToUnrealAngularVelocity(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere angular velocity"), Actual, Expected);
	}

	// Mass.
	{
		Test.TestEqual(TEXT("Sphere mass"), SphereBody->Mass, 5.00000000000000000000e+02f);
	}

	// Inertia tensor diagonal.
	{
		FVector Actual = SphereBody->GetPrincipalInertia();
		FVector Expected(
			1.00000000000000000000e+02f, 2.00000000000000000000e+02f, 3.00000000000000000000e+02f);
		Test.TestEqual(TEXT("Sphere inertia tensor diagonal"), Actual, Expected);
	}

	// Motion control.
	{
		EAGX_MotionControl Actual = SphereBody->MotionControl;
		EAGX_MotionControl Expected = EAGX_MotionControl::MC_DYNAMICS;
		Test.TestEqual(TEXT("Sphere motion control"), Actual, Expected);
	}

	// Transform root component.
	{
		Test.TestEqual(
			TEXT("Sphere transform target"), SphereBody->TransformTarget,
			EAGX_TransformTarget::TT_SELF);
	}

	// Radius.
	{
		float Actual = SphereShape->Radius;
		float ExpectedAgx = 5.00000000000000000000e-01f;
		float Expected = AgxToUnrealDistance(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere radius"), Actual, Expected);
	}

	// Imported objects don't get a native AGX Dynamics representation immediately when imported
	// into Unreal Editor but this unit test is run in Game mode which means that BeginPlay is
	// called on an Actor as soon as it is created, and Actors which have had BeginPlay called will
	// call BeginPlay on any registered Component, with UActorComponent::RegisterComponent,
	// immediately. Which creates the AGX Dynamics native object.
	Test.TestTrue(TEXT("Sphere has native"), SphereBody->HasNative());

	// The body should have been created in the test world.
	Test.TestEqual(TEXT("Sphere world"), SphereBody->GetWorld(), Test.World);

	// Publish the important bits to the rest of the test.
	Test.SphereBody = SphereBody;
	Test.StartPosition = SphereBody->GetComponentLocation();
	Test.StartVelocity = SphereBody->Velocity;

	return true;
}

bool FStoreInitialTimes::Update()
{
	Test.StartUnrealTime = Test.World->GetTimeSeconds();
	Test.StartAgxTime = Test.StartUnrealTime;
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(Test.World);
	Simulation->SetTimeStamp(Test.StartUnrealTime);
	Simulation->StepMode = SmCatchUpImmediately;
	return true;
}

bool FStoreResultingTimes::Update()
{
	Test.EndUnrealTime = Test.World->GetTimeSeconds();
	Test.EndAgxTime = UAGX_Simulation::GetFrom(Test.World)->GetTimeStamp();
	return true;
}

bool FCheckSphereHasMoved::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.SphereBody == nullptr)
	{
		return true;
	}

	FVector EndPosition = Test.SphereBody->GetComponentLocation();
	FVector EndVelocity = Test.SphereBody->Velocity;
	float Duration = Test.EndAgxTime - Test.StartAgxTime;

	// Velocity constant only for X and Y directions. Z has gravity.
	for (int32 I : {0, 1})
	{
		float ExpectedDisplacement = EndVelocity[I] * Duration;
		float ExpectedPosition = Test.StartPosition[I] + ExpectedDisplacement;
		float ActualPosition = EndPosition[I];
		Test.TestEqual(
			"Body should move according to velocity.", ActualPosition, ExpectedPosition,
			RelativeTolerance(ExpectedPosition, KINDA_SMALL_NUMBER));
	}

	// Position test for Z.
	{
		float StartVelocity = Test.StartVelocity.Z;
		float StartPosition = Test.StartPosition.Z;
		float Acceleration = UAGX_Simulation::GetFrom(Test.World)->UniformGravity.Z;
		// The familiar Xt = X0 + V0 * t + 1/2 * a * t^2.
		float ExpectedPosition =
			StartPosition + StartVelocity * Duration + 0.5f * Acceleration * Duration * Duration;
		float ActualPosition = EndPosition.Z;
		/// @todo Not sure why the relative tolerance must be so large here. Maybe gravity mismatch?
		/// There is also the SPOOK/leapfrog integration formulation that makes free-fall in gravity
		/// a bit less straight-forward. Perhaps that is enough to explain the difference.
		Test.TestEqual(
			"Velocity in the Z direction should be subject to gravity.", ActualPosition,
			ExpectedPosition, RelativeTolerance(ExpectedPosition, 0.003f));
	}

	return true;
}

bool FClearSingleSphereImportedCommand::Update()
{
	UWorld* World = AgxAutomationCommon::GetTestWorld();
	if (World == nullptr || Contents == nullptr)
	{
		return true;
	}
	World->DestroyActor(Contents);
	Contents = nullptr;
	return true;
}

//
// MotionControl test starts here.
//

class FImporterToSingleActor_MotionControlTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckMotionControlImportedCommand, FImporterToSingleActor_MotionControlTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearMotionControlImportedCommand, FImporterToSingleActor_MotionControlTest&, Test);

class FImporterToSingleActor_MotionControlTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_MotionControlTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_MotionControlTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.MotionControl"))
	{
	}

public:
	AActor* Contents = nullptr; // <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("motion_control_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckMotionControlImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearMotionControlImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_MotionControlTest FImporterToSingleActor_MotionControlTest;
}

bool FCheckMotionControlImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import MotionControl test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 7);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* StaticBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("StaticBody"));
	UAGX_SphereShapeComponent* StaticShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("StaticShape"));
	UAGX_RigidBodyComponent* KinematicsBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("KinematicBody"));
	UAGX_SphereShapeComponent* KinematicsShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("KinematicShape"));
	UAGX_RigidBodyComponent* DynamicsBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("DynamicBody"));
	UAGX_SphereShapeComponent* DynamicsShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("DynamicShape"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("StaticBody"), StaticBody);
	Test.TestNotNull(TEXT("StaticShape"), StaticShape);
	Test.TestNotNull(TEXT("KinematicBody"), KinematicsBody);
	Test.TestNotNull(TEXT("KinematicShape"), KinematicsShape);
	Test.TestNotNull(TEXT("DynamicBody"), DynamicsBody);
	Test.TestNotNull(TEXT("DynamicShape"), DynamicsShape);
	if (SceneRoot == nullptr || StaticBody == nullptr || StaticShape == nullptr ||
		KinematicsBody == nullptr || KinematicsShape == nullptr || DynamicsBody == nullptr ||
		DynamicsShape == nullptr)
	{
		Test.AddError("A required component wasn't found in the imported actor. Cannot continue");
		return true;
	}

	Test.TestEqual(
		TEXT("Static body motion control"), StaticBody->MotionControl,
		EAGX_MotionControl::MC_STATIC);
	Test.TestEqual(
		TEXT("Kinematic body motion control"), KinematicsBody->MotionControl,
		EAGX_MotionControl::MC_KINEMATICS);
	Test.TestEqual(
		TEXT("Dynamic body motion control"), DynamicsBody->MotionControl,
		EAGX_MotionControl::MC_DYNAMICS);

#if 0
	// We would like this to be Static, but setting it in UAGX_RigidBodyComponent::CopyFrom during
	// import of an AGX Dynamics archive causes the move widget in Unreal Editor to break. Static
	// bodies doesn't follow the Actor's RootComponent. For now we set Mobility to Movable even for
	// static bodies. This comes with a rendering performance cost since baked lighting can't be
	// used anymore, and possibly other reasons as well. I'm guessing that we're supposed to do
	// something when changing Mobility to make it update everywhere, or perhaps we're setting it
	// at the wrong time. Not sure, more investigation needed.
	Test.TestEqual(TEXT("Static body mobility"), StaticBody->Mobility, EComponentMobility::Static);
#else
	Test.TestEqual(TEXT("Static body mobility"), StaticBody->Mobility, EComponentMobility::Movable);
#endif
	Test.TestEqual(
		TEXT("Kinematic body mobility"), KinematicsBody->Mobility, EComponentMobility::Movable);
	Test.TestEqual(
		TEXT("Dynamic body mobility"), DynamicsBody->Mobility, EComponentMobility::Movable);

	return true;
}

bool FClearMotionControlImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}
	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}
	return true;
}

//
// SimpleTrimesh test starts here.
//

class FImporterToSingleActor_SimpleTrimeshTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSimpleTrimeshImportedCommand, FImporterToSingleActor_SimpleTrimeshTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSimpleTrimeshImportedCommand, FImporterToSingleActor_SimpleTrimeshTest&, Test);

class FImporterToSingleActor_SimpleTrimeshTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_SimpleTrimeshTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_SimpleTrimeshTest"),
			  TEXT("AGXUnreal.Game.ImporterToSingleActor.SimpleTrimesh"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NO_AGX(false)
		BAIL_TEST_IF_NO_WORLD(false)
		BAIL_TEST_IF_WORLDS_MISMATCH(false)
		World = AgxAutomationCommon::GetTestWorld();
		Simulation = UAGX_Simulation::GetFrom(World);

		// See comment in FImporterToSingleActor_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("simple_trimesh_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSimpleTrimeshImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSimpleTrimeshImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_SimpleTrimeshTest ImporterToSingleActor_SimpleTrimeshTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script simple_trimesh.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckSimpleTrimeshImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import SimpleTrimesh test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 4);

	// Get the components we know should be there.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* TrimeshBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("TrimeshBody"));
	UAGX_TrimeshShapeComponent* TrimeshShape =
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("TrimeshGeometry"));
	UStaticMeshComponent* StaticMesh =
		GetByName<UStaticMeshComponent>(Components, TEXT("simple_trimesh"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("TrimeshBody"), TrimeshBody);
	Test.TestNotNull(TEXT("TrimeshShape"), TrimeshShape);
	Test.TestNotNull(TEXT("StaticMesh"), StaticMesh);
	if (TrimeshBody == nullptr || TrimeshShape == nullptr || StaticMesh == nullptr)
	{
		Test.AddError("A required component wasn't found in the imported actor. Cannot continue.");
		return true;
	}

	const TArray<USceneComponent*>& Children = TrimeshShape->GetAttachChildren();
	Test.TestEqual(TEXT("TrimeshShape child components"), Children.Num(), 1);
	if (Children.Num() != 1)
	{
		return true;
	}
	USceneComponent* Child = Children[0];
	Test.TestNotNull(TEXT("Child"), Child);
	if (Child == nullptr)
	{
		return true;
	}
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child);
	Test.TestNotNull(TEXT("Trimesh asset"), Mesh);
	if (Mesh == nullptr)
	{
		return true;
	}
	Test.TestEqual(TEXT("The StaticMesh should be a child of the TrimeshShape"), Mesh, StaticMesh);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearSimpleTrimeshImportedCommand::Update()
{
	if (Test.World == nullptr || Test.Contents == nullptr)
	{
		return true;
	}
	Test.World->DestroyActor(Test.Contents);

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {TEXT("StaticMesh"), TEXT("simple_trimesh.uasset")};
	AgxAutomationCommon::DeleteImportDirectory(TEXT("simple_trimesh_build"), ExpectedFiles);

	return true;
}

//
// RenderMaterial test starts here.
//

class FImporterToSingleActor_RenderMaterialTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRenderMaterialImportedCommand, FImporterToSingleActor_RenderMaterialTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRenderMaterialImportedCommand, FImporterToSingleActor_RenderMaterialTest&, Test);

class FImporterToSingleActor_RenderMaterialTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_RenderMaterialTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_RenderMaterialTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.RenderMaterial"))
	{
	}

public:
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("render_materials_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRenderMaterialImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRenderMaterialImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_RenderMaterialTest ImporterToSingleActor_RenderMaterialTest;
}

namespace
{
	/// \todo These are also in AGX_ImportUtilities, but I get linker errors when using them even
	/// though AGXUnrealEditor is in the modules list in AGXUnrealTest.Build.cs. Copying the
	/// code here for now.

	FLinearColor SRGBToLinear(const FVector4& SRGB)
	{
		FColor SRGBBytes(
			static_cast<uint8>(SRGB.X * 255.0f), static_cast<uint8>(SRGB.Y * 255.0f),
			static_cast<uint8>(SRGB.Z * 255.0f), static_cast<uint8>(SRGB.W * 255.0f));
		return {SRGBBytes};
	}

	FVector4 LinearToSRGB(const FLinearColor& Linear)
	{
		FColor SRGBBytes = Linear.ToFColor(true);
		return FVector4(
			static_cast<float>(SRGBBytes.R) / 255.0f, static_cast<float>(SRGBBytes.G) / 255.0f,
			static_cast<float>(SRGBBytes.B) / 255.0f, static_cast<float>(SRGBBytes.A) / 255.0f);
	}
}

namespace CheckRenderMaterialImportedCommand_helpers
{
	void TestScalar(
		UMaterialInterface& Material, const TCHAR* ParameterName, float Expected,
		FAutomationTestBase& Test)
	{
		FMaterialParameterInfo Info;
		Info.Name = ParameterName;
		float Actual;
		if (!Material.GetScalarParameterValue(Info, Actual, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get parameter '%s' for material '%s'."), ParameterName,
				*Material.GetName()));
			return;
		}
		Test.TestEqual(
			*FString::Printf(TEXT("%s in %s"), ParameterName, *Material.GetName()), Actual,
			Expected);
	}

	void TestColor(
		UMaterialInterface& Material, const TCHAR* ParameterName, const FVector4& Expected,
		FAutomationTestBase& Test)
	{
		FMaterialParameterInfo Info;
		Info.Name = ParameterName;
		FLinearColor ActualLinear;
		if (!Material.GetVectorParameterValue(Info, ActualLinear, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get parameter '%s' from material '%s'."), ParameterName,
				*Material.GetName()));
			return;
		}

		FVector4 Actual = LinearToSRGB(ActualLinear);
		float Tolerance = 1.0f / 255.0f; // This is all the precision we have in a byte.
		AgxAutomationCommon::TestEqual(
			Test, *FString::Printf(TEXT("%s in %s"), ParameterName, *Material.GetName()), Actual,
			Expected, Tolerance);
	}

	struct FMaterialParameters
	{
		// These are the default material properties in AGX Dynamics. Each test override a subset of
		// these.
		FVector4 Ambient = LinearToSRGB({0.01f, 0.0028806f, 0.0f, 1.0f});
		FVector4 Diffuse = LinearToSRGB({0.8962694f, 0.258183f, 0.0f, 1.0f});
		FVector4 Emissive = LinearToSRGB({0.0f, 0.0f, 0.0f, 1.0f});
		float Shininess {0.0f};
	};

	void TestMaterial(
		UAGX_SphereShapeComponent& Sphere, const FMaterialParameters& Parameters,
		FAutomationTestBase& Test)
	{
		UMaterialInterface* Material = Sphere.GetMaterial(0);
		if (Material == nullptr)
		{
			Test.AddError(
				FString::Printf(TEXT("Sphere '%s' does not have a material."), *Sphere.GetName()));
			return;
		}
		TestColor(*Material, TEXT("Ambient"), Parameters.Ambient, Test);
		TestColor(*Material, TEXT("Diffuse"), Parameters.Diffuse, Test);
		TestColor(*Material, TEXT("Emissive"), Parameters.Emissive, Test);
		TestScalar(*Material, TEXT("Shininess"), Parameters.Shininess, Test);
	}
}

bool FCheckRenderMaterialImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	using namespace CheckRenderMaterialImportedCommand_helpers;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RenderMaterial test scene: No content created."));
		return true;
	}

	// Get all the imported components. The test for the number of components is a safety check.
	// It should be updated whenever the test scene is changed.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 16);

// Enable this to see the names of the components that was imported. Useful when adding new stuff
// to the archive.
#if 0
	UE_LOG(LogAGX, Warning, TEXT("Imported the following components:"));
	for (const UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *Component->GetName());
	}
#endif

	auto GetSphere = [&Components](const TCHAR* Name) -> UAGX_SphereShapeComponent* {
		return GetByName<UAGX_SphereShapeComponent>(Components, Name);
	};

	// Get the components we know should be there.
	/// @todo Some of these get auto-generated names because of name conflicts. Happens every time a
	/// agxCollide::Geometry contains more than once agxCollide::Shape since the name lives in the
	/// Geometry. So far the generated names have been consistent between runs, but I'm not sure if
	/// we're guaranteed that. Especially if we run multiple tests in the same invocation of the
	/// editor. The fix is to fetch objects based on UUID/GUID instead of names.
	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* Body =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("RenderMaterialBody"));
	UAGX_SphereShapeComponent* Ambient = GetSphere(TEXT("AmbientGeometry"));
	UAGX_SphereShapeComponent* Diffuse = GetSphere(TEXT("DiffuseGeometry"));
	UAGX_SphereShapeComponent* Emissive = GetSphere(TEXT("EmissiveGeometry"));
	UAGX_SphereShapeComponent* Shininess = GetSphere(TEXT("ShininessGeometry"));
	UAGX_SphereShapeComponent* AmbientDiffuse = GetSphere(TEXT("AmbientDiffuseGeometry"));
	UAGX_SphereShapeComponent* AmbientEmissive = GetSphere(TEXT("AmbientEmissiveGeometry"));
	UAGX_SphereShapeComponent* DiffuseShininessLow = GetSphere(TEXT("DiffuseShininessLowGeometry"));
	UAGX_SphereShapeComponent* DiffuseShininessHigh =
		GetSphere(TEXT("DiffuseShininessHighGeometry"));
	UAGX_SphereShapeComponent* SharedSphere1 = GetSphere(TEXT("SharedGeometry"));
	UAGX_SphereShapeComponent* SharedSphere2 = GetSphere(TEXT("SharedGeometry_10"));
	UAGX_SphereShapeComponent* NameConflictSphere1 = GetSphere(TEXT("MaterialNameConflict"));
	UAGX_SphereShapeComponent* NameConflictSphere2 = GetSphere(TEXT("MaterialNameConflict_13"));
	UAGX_SphereShapeComponent* VisibleSphere = GetSphere(TEXT("VisibleSphere"));
	UAGX_SphereShapeComponent* InvisibleSphere = GetSphere(TEXT("InvisibleSphere"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("Body"), Body);
	Test.TestNotNull(TEXT("Ambient"), Ambient);
	Test.TestNotNull(TEXT("Diffuse"), Diffuse);
	Test.TestNotNull(TEXT("Emissive"), Emissive);
	Test.TestNotNull(TEXT("Shininess"), Shininess);
	Test.TestNotNull(TEXT("AmbientDiffuse"), AmbientDiffuse);
	Test.TestNotNull(TEXT("AmbientEmissive"), AmbientEmissive);
	Test.TestNotNull(TEXT("DiffuseShininessLow"), DiffuseShininessLow);
	Test.TestNotNull(TEXT("DiffuseShininessHigh"), DiffuseShininessHigh);
	Test.TestNotNull(TEXT("SharedSphere1"), SharedSphere1);
	Test.TestNotNull(TEXT("SharedSphere2"), SharedSphere2);
	Test.TestNotNull(TEXT("NameConflictSphere1"), NameConflictSphere1);
	Test.TestNotNull(TEXT("NameConflictSphere2"), NameConflictSphere2);
	Test.TestNotNull(TEXT("VisibleSphere"), VisibleSphere);
	Test.TestNotNull(TEXT("InvisibleSphere"), InvisibleSphere);

	if (SceneRoot == nullptr || Body == nullptr || Ambient == nullptr || Diffuse == nullptr ||
		Emissive == nullptr || Shininess == nullptr || AmbientDiffuse == nullptr ||
		AmbientEmissive == nullptr || DiffuseShininessLow == nullptr ||
		DiffuseShininessHigh == nullptr || SharedSphere1 == nullptr || SharedSphere2 == nullptr ||
		NameConflictSphere1 == nullptr || NameConflictSphere2 == nullptr ||
		VisibleSphere == nullptr || InvisibleSphere == nullptr)
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	// Ambient.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.32f, 0.85f, 0.21f, 1.0f);
		TestMaterial(*Ambient, Parameters, Test);
	}
	// Diffuse.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.80f, 0.34f, 0.21f, 1.0f);
		TestMaterial(*Diffuse, Parameters, Test);
	}
	// Emissive.
	{
		FMaterialParameters Parameters;
		Parameters.Emissive = FVector4(0.98f, 0.94f, 0.76f, 1.0f);
		TestMaterial(*Emissive, Parameters, Test);
	}
	// AmbientDiffuse.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.81f, 0.34f, 0.26f, 1.0f);
		Parameters.Diffuse = FVector4(0.32f, 0.28f, 0.67f, 1.0f);
		TestMaterial(*AmbientDiffuse, Parameters, Test);
	}
	// AmbientEmissive.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = FVector4(0.32f, 0.34f, 0.54f, 1.0f);
		Parameters.Emissive = FVector4(0.21f, 0.17f, 0.23f, 1.0f);
		TestMaterial(*AmbientEmissive, Parameters, Test);
	}
	// DiffuseShininessLow.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.65f, 0.74f, 0.48f, 1.0f);
		Parameters.Shininess = 0.0f;
		TestMaterial(*DiffuseShininessLow, Parameters, Test);
	}
	// DiffuseShininessHigh.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = FVector4(0.65f, 0.74f, 0.48f, 1.0f);
		Parameters.Shininess = 1.0f;
		TestMaterial(*DiffuseShininessHigh, Parameters, Test);
	}
	// Shared.
	{
		const UMaterialInterface* const Material1 = SharedSphere1->GetMaterial(0);
		const UMaterialInterface* const Material2 = SharedSphere2->GetMaterial(0);
		Test.TestNotNull(TEXT("SharedSphere1 material"), Material1);
		Test.TestNotNull(TEXT("SharedSphere2 material"), Material2);
		Test.TestEqual(TEXT("SharedSphere materials"), Material1, Material2);
	}
	// NameConflict.
	{
		const UMaterialInterface* const Material1 = NameConflictSphere1->GetMaterial(0);
		const UMaterialInterface* const Material2 = NameConflictSphere2->GetMaterial(0);
		Test.TestNotEqual(TEXT("Name conflict materials"), Material1, Material2);
// AGX Dynamics does not currently store render material names in archives. The importer always
// reads empty strings. Enable these tests if/when render material names are added to serialization.
#if 0
		Test.TestEqual(TEXT("Conflict name"), Material1->GetName(), FString(TEXT("NameConflict")));
		Test.TestEqual(TEXT("Conflict name"), Material2->GetName(), FString(TEXT("NameConflict")));
#endif
		FMaterialParameters Parameters1;
		Parameters1.Shininess = 0.30f;
		TestMaterial(*NameConflictSphere1, Parameters1, Test);
		FMaterialParameters Parameters2;
		Parameters2.Shininess = 0.99f;
		TestMaterial(*NameConflictSphere2, Parameters2, Test);
	}
	// ShouldRender.
	{
		Test.TestTrue(TEXT("VisibleSphere Visible flag"), VisibleSphere->GetVisibleFlag());
		Test.TestFalse(TEXT("InvisibleSphere Visible flag"), InvisibleSphere->GetVisibleFlag());
	}

	return true;
}

bool FClearRenderMaterialImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Files that are created by the test and thus safe to remove. The GUID values may make this
	// test cumbersome to update since they will change every time the AGX Dynamics archive is
	// regenerated. Consider either adding wildcard support to DeleteImportDirectory or assign
	// names to the render materials in the source .agxPy file.
	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("RenderMaterial"),
		TEXT("RenderMaterial_0371489EAC6B66145E4DAEEFA9B4B6BC.uasset"),
		TEXT("RenderMaterial_29524C99D524BAD2D65ED4EEA25BE374.uasset"),
		TEXT("RenderMaterial_2C1C438F862A19CB2E1618C7B73F3D2E.uasset"),
		TEXT("RenderMaterial_5928446AC6D455AF9FFF7C9BACFEA7B4.uasset"),
		TEXT("RenderMaterial_61674EB1E1568AF222ED9F4C8840B70B.uasset"),
		TEXT("RenderMaterial_87A0492CA6F19D8C434FCF80A8D8A9FF.uasset"),
		TEXT("RenderMaterial_8DAE4BA30D3EA367D30FC0699240725B.uasset"),
		TEXT("RenderMaterial_CF1F4CAD342CCF1863EFCC4093609BA4.uasset"),
		TEXT("RenderMaterial_D28B4DA2420D497375E97FA4B5365ADC.uasset"),
		TEXT("RenderMaterial_D4164CD0F7CD4A1706FC2B7E85ACE0F0.uasset"),
		TEXT("RenderMaterial_F41041270F6DCA5B49388F72978AFC64.uasset")};

	AgxAutomationCommon::DeleteImportDirectory(TEXT("render_materials_build"), ExpectedFiles);

	return true;
}

//
// Render Data test starts here.
//

class FImporterToSingleActor_RenderDataTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRenderDataImportedCommand, FImporterToSingleActor_RenderDataTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRenderDataImportedCommand, FImporterToSingleActor_RenderDataTest&, Test);

class FImporterToSingleActor_RenderDataTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_RenderDataTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_RenderDataTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.RenderData"))
	{
	}

public:
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("render_data_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRenderDataImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRenderDataImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_RenderDataTest ImporterToSingleActor_RenderDataTest;
}

bool FCheckRenderDataImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RenderMaterial test scene: No content created."));
		return true;
	}

	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	// Root(1), Rigid Body(2), Shape(3), Static Mesh(4).
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 4);

	// Enable this to see the names of the components that was imported. Useful when adding new
	// stuff to the archive.
#if 1
	UE_LOG(LogAGX, Warning, TEXT("Imported the following components:"));
	for (const UActorComponent* Component : Components)
	{
		UE_LOG(LogAGX, Warning, TEXT("  %s"), *Component->GetName());
	}
#endif

	UAGX_SphereShapeComponent* Sphere =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("Render Data Geometry"));
	UStaticMeshComponent* Mesh = GetByName<UStaticMeshComponent>(
		Components, TEXT("RenderMesh_F42A4C942C9E27E9B873D061BAF66764"));

	Test.TestNotNull(TEXT("Sphere"), Sphere);
	Test.TestNotNull(TEXT("Mesh"), Mesh);

	if (IsAnyNullptr(Sphere, Mesh))
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	Test.TestTrue(
		TEXT("The mesh should be a child of the sphere"), Mesh->GetAttachParent() == Sphere);

	return true;
}

bool FClearRenderDataImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	// Files that are created by the test and thus safe to remove. The GUID values may make this
	// test cumbersome to update since they will change every time the AGX Dynamics archive is
	// regenerated. Consider either adding wildcard support to DeleteImportDirectory or assign
	// names to the render materials in the source .agxPy file.
	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("RenderMesh"), TEXT("RenderMesh_F42A4C942C9E27E9B873D061BAF66764.uasset")};

	AgxAutomationCommon::DeleteImportDirectory(TEXT("render_data_build"), ExpectedFiles);

	return true;
}

//
// CollisionGroups test starts here.
//

class FImporterToSingleActor_CollisionGroupsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckCollisionGroupsImportedCommand, FImporterToSingleActor_CollisionGroupsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearCollisionGroupsImportedCommand, FImporterToSingleActor_CollisionGroupsTest&, Test);

class FImporterToSingleActor_CollisionGroupsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_CollisionGroupsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_CollisionGroupsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.CollisionGroups"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("collision_groups_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckCollisionGroupsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearCollisionGroupsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_CollisionGroupsTest ImporterToSingleActor_CollisionGroupsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script collision_groups.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckCollisionGroupsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import CollisionGroups test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 18);

	auto GetBox = [&Components](
					  const TCHAR* Name,
					  TArray<UAGX_BoxShapeComponent*>& OutArr) -> UAGX_BoxShapeComponent*
	{
		UAGX_BoxShapeComponent* Box = GetByName<UAGX_BoxShapeComponent>(Components, Name);
		OutArr.Add(Box);
		return Box;
	};

	auto GetBody = [&Components](
					   const TCHAR* Name,
					   TArray<UAGX_RigidBodyComponent*>& OutArr) -> UAGX_RigidBodyComponent*
	{
		UAGX_RigidBodyComponent* Rb = GetByName<UAGX_RigidBodyComponent>(Components, Name);
		OutArr.Add(Rb);
		return Rb;
	};

	TArray<UAGX_RigidBodyComponent*> RbArr;
	TArray<UAGX_BoxShapeComponent*> BoxArr;
	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* rb_0_brown = GetBody(TEXT("rb_0_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_0_brown = GetBox(TEXT("geom_0_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_1_brown = GetBody(TEXT("rb_left_1_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_left_1_brown = GetBox(TEXT("geom_left_1_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_right_1_orange = GetBody(TEXT("rb_right_1_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_right_1_orange = GetBox(TEXT("geom_right_1_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_2_orange = GetBody(TEXT("rb_left_2_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_left_2_orange = GetBox(TEXT("geom_left_2_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_right_2_orange = GetBody(TEXT("rb_right_2_orange"), RbArr);
	UAGX_BoxShapeComponent* geom_right_2_orange = GetBox(TEXT("geom_right_2_orange"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_3_brown = GetBody(TEXT("rb_left_3_brown"), RbArr);
	UAGX_BoxShapeComponent* geom_left_3_brown = GetBox(TEXT("geom_left_3_brown"), BoxArr);
	UAGX_RigidBodyComponent* rb_4_blue = GetBody(TEXT("rb_4_blue"), RbArr);
	UAGX_BoxShapeComponent* geom_4_blue = GetBox(TEXT("geom_4_blue"), BoxArr);
	UAGX_RigidBodyComponent* rb_left_5_blue = GetBody(TEXT("rb_left_5_blue"), RbArr);
	UAGX_BoxShapeComponent* geom_left_5_blue = GetBox(TEXT("geom_left_5_blue"), BoxArr);
	UAGX_CollisionGroupDisablerComponent* AGX_CollisionGroupDisabler =
		GetByName<UAGX_CollisionGroupDisablerComponent>(
			Components, TEXT("AGX_CollisionGroupDisabler"));

	Test.TestEqual(TEXT("Number of Rigid Bodies"), RbArr.Num(), 8);
	Test.TestEqual(TEXT("Number of Box Shapes"), BoxArr.Num(), 8);

	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("AGX_CollisionGroupDisabler"), AGX_CollisionGroupDisabler);

	for (auto& R : RbArr)
	{
		Test.TestNotNull(TEXT("Rigid Body"), R);
		if (R == nullptr)
		{
			Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
			return true;
		}
	}

	for (auto& B : BoxArr)
	{
		Test.TestNotNull(TEXT("Box Shape"), B);
		if (B == nullptr)
		{
			Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
			return true;
		}

		Test.TestEqual(TEXT("Number of Collision groups"), B->CollisionGroups.Num(), 1);
		if (B->CollisionGroups.Num() != 1)
		{
			Test.AddError(TEXT("Wrong number of collision groups, cannot continue."));
			return true;
		}
	}

	Test.TestEqual(
		TEXT("Collision group name"), geom_0_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_1_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_right_1_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_2_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_right_2_orange->CollisionGroups[0].IsEqual("5"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_3_brown->CollisionGroups[0].IsEqual("A"), true);
	Test.TestEqual(TEXT("Collision grp name"), geom_4_blue->CollisionGroups[0].IsEqual("b"), true);
	Test.TestEqual(
		TEXT("Collision grp name"), geom_left_5_blue->CollisionGroups[0].IsEqual("b"), true);

	Test.TestEqual(
		TEXT("Number of Collision group pairs"),
		AGX_CollisionGroupDisabler->DisabledCollisionGroupPairs.Num(), 3);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("A"), FName("b")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("b"), FName("b")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("5"), FName("5")), true);

	Test.TestEqual(
		TEXT("Pair collision disabled"),
		AGX_CollisionGroupDisabler->IsCollisionGroupPairDisabled(FName("A"), FName("A")), false);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearCollisionGroupsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

	return true;
}

//
// GeometrySensors test starts here.
//

class FImporterToSingleActor_GeometrySensorsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckGeometrySensorsImportedCommand, FImporterToSingleActor_GeometrySensorsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearGeometrySensorsImportedCommand, FImporterToSingleActor_GeometrySensorsTest&, Test);

class FImporterToSingleActor_GeometrySensorsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_GeometrySensorsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_GeometrySensorsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.GeometrySensors"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("geometry_sensors_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckGeometrySensorsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearGeometrySensorsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_GeometrySensorsTest ImporterToSingleActor_GeometrySensorsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script geometry_sensors.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckGeometrySensorsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import GeometrySensors test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// Three Rigid Bodies, three Geometries and one Default Scene Root.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 7);

	UAGX_SphereShapeComponent* BoolSensor =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("boolSensor"));

	UAGX_CylinderShapeComponent* ContactsSensor =
		GetByName<UAGX_CylinderShapeComponent>(Components, TEXT("contactsSensor"));

	UAGX_BoxShapeComponent* NotASensor =
		GetByName<UAGX_BoxShapeComponent>(Components, TEXT("notASensor"));

	Test.TestNotNull(TEXT("boolSensor"), BoolSensor);
	Test.TestNotNull(TEXT("contactsSensor"), ContactsSensor);
	Test.TestNotNull(TEXT("notASensor"), NotASensor);

	if (BoolSensor == nullptr || ContactsSensor == nullptr || NotASensor == nullptr)
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	// Test the bIsSensor property.
	Test.TestEqual(TEXT("Is Sensor property"), BoolSensor->bIsSensor, true);
	Test.TestEqual(TEXT("Is Sensor property"), ContactsSensor->bIsSensor, true);
	Test.TestEqual(TEXT("Is Sensor property"), NotASensor->bIsSensor, false);

	// Test the SensorType property (only relevant for sensor geometries).
	Test.TestEqual(
		TEXT("Sensor type property"), BoolSensor->SensorType == EAGX_ShapeSensorType::BooleanSensor,
		true);

	Test.TestEqual(
		TEXT("Sensor type property"),
		ContactsSensor->SensorType == EAGX_ShapeSensorType::ContactsSensor, true);

	// Test the Materials applied after import.
	const auto BoolSensorMaterials = BoolSensor->GetMaterials();
	Test.TestEqual(
		TEXT("Sensor Material"),
		(BoolSensorMaterials.Num() == 1 && BoolSensorMaterials[0] != nullptr &&
		 BoolSensorMaterials[0]->GetName() == "M_SensorMaterial"),
		true);

	const auto ContactsSensorMaterials = ContactsSensor->GetMaterials();
	Test.TestEqual(
		TEXT("Sensor Material"),
		(ContactsSensorMaterials.Num() == 1 && ContactsSensorMaterials[0] != nullptr &&
		 ContactsSensorMaterials[0]->GetName() == "M_SensorMaterial"),
		true);

	const auto NotASensorMaterials = NotASensor->GetMaterials();
	Test.TestEqual(
		TEXT("Default Material"),
		(NotASensorMaterials.Num() == 1 && NotASensorMaterials[0] != nullptr &&
		 NotASensorMaterials[0]->GetName() == "M_ImportedBase"),
		true);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearGeometrySensorsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

	return true;
}

#if AGX_TEST_WIRE_IMPORT
//
// Wire test starts here.
//

class FImporterToSingleActor_WireTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckWireImportedCommand, FImporterToSingleActor_WireTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearWireImportedCommand, FImporterToSingleActor_WireTest&, Test);

class FImporterToSingleActor_WireTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_WireTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_WireTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.Wire"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("wire_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckWireImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearWireImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_WireTest ImporterToSingleActor_WireTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script wire.agxPy.
 *
 * @return True when the check is complete. Never returns false.
 */
bool FCheckWireImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import Wire test scene: No content created"));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// A Wire (1) and its icon (2), three Rigid Bodies (5), three Shapes (8), a Collision Group
	// Disabler (9), and a Default Scene Root (10).
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 10);
	if (Components.Num() != 10)
	{
		UE_LOG(LogAGX, Warning, TEXT("Found the following components:"));
		for (auto Component : Components)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  %s: %s"), *Component->GetName(),
				*Component->GetClass()->GetName());
		}
	}

	UAGX_WireComponent* Wire = GetByName<UAGX_WireComponent>(Components, TEXT("Wire"));
	UAGX_RigidBodyComponent* WinchBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("Winch Body"));
	UAGX_CylinderShapeComponent* WinchShape =
		GetByName<UAGX_CylinderShapeComponent>(Components, TEXT("Winch Shape"));
	UAGX_RigidBodyComponent* EyeBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("Eye Body"));
	UAGX_SphereShapeComponent* EyeShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("Eye Shape"));
	UAGX_RigidBodyComponent* BeadBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("Bead Body"));
	UAGX_CapsuleShapeComponent* BeadShape =
		GetByName<UAGX_CapsuleShapeComponent>(Components, TEXT("Bead Shape"));

	Test.TestNotNull(TEXT("Wire"), Wire);
	Test.TestNotNull(TEXT("Winch Body"), WinchBody);
	Test.TestNotNull(TEXT("Winch Shape"), WinchShape);
	Test.TestNotNull(TEXT("Eye Body"), EyeBody);
	Test.TestNotNull(TEXT("Eye Shape"), EyeShape);
	Test.TestNotNull(TEXT("Bead Body"), BeadBody);
	Test.TestNotNull(TEXT("Bead Shape"), BeadShape);

	if (IsAnyNullptr(Wire, WinchBody, WinchShape, EyeBody, EyeShape, BeadBody, BeadShape))
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	const float Radius = CentimeterToMeter(Wire->Radius);
	const float Resolution = 1.0f / CentimeterToMeter(Wire->MinSegmentLength);
	FAGX_WireWinch& Winch = Wire->OwnedBeginWinch;
	const double PulledInLength = CentimeterToMeter(Winch.GetPulledInLength());
	const double Speed = CentimeterToMeter(Winch.GetTargetSpeed());
	const FVector WinchLocation {
		CentimeterToMeter(Winch.Location.X), CentimeterToMeter(Winch.Location.Y),
		CentimeterToMeter(Winch.Location.Z)};

	// Note negated Y, since AGX Dynamics is right-handed and Unreal Engine is left-handed.
	const FVector ExpectedDirection = FVector(0.696526, -0.398015, 0.597022).GetUnsafeNormal();
	const FRotator ExpectedRotation = ExpectedDirection.Rotation();

	Test.TestEqual(TEXT("Radius"), Radius, 0.05f);
	Test.TestEqual(TEXT("Resolution"), Resolution, 3.0f);
	Test.TestEqual(TEXT("Begin winch type"), Wire->BeginWinchType, EWireWinchOwnerType::Wire);
	Test.TestEqual(TEXT("Pulled in length"), PulledInLength, 10.0);
	Test.TestEqual(TEXT("Speed"), Speed, 0.5);
	Test.TestEqual(TEXT("Num route nodes"), Wire->RouteNodes.Num(), 23);

	Test.TestEqual(TEXT("GetMotorForceRangeMin"), Winch.GetMotorForceRangeMin(), -10.0);
	Test.TestEqual(TEXT("GetMotorForceRangeMax"), Winch.GetMotorForceRangeMax(), 20.0);
	Test.TestEqual(TEXT("GetBrakeForceRangeMin"), Winch.GetBrakeForceRangeMin(), -100.0);
	Test.TestEqual(TEXT("GetBrakeForceRangeMax"), Winch.GetBrakeForceRangeMax(), 200.0);
	Test.TestEqual(TEXT("IsMotorEnabled"), Winch.IsMotorEnabled(), true);
	Test.TestEqual(TEXT("IsBrakeEnabled"), Winch.IsBrakeEnabled(), true);
	Test.TestEqual(TEXT("Location"), WinchLocation, FVector(0.6f, 0.0f, 0.0f));
	// Somewhat unclear why we need such a big tolerance here. The value is created through
	// computation, both in Python, during import, and in this test, but still 1e-3 is very big.
	AgxAutomationCommon::TestEqual(Test, TEXT("Rotation"), Winch.Rotation, ExpectedRotation, 1e-3f);
	Test.TestEqual(TEXT("bAutoFeed"), Winch.bAutoFeed, false);

	auto RangeHasType =
		[this, &Nodes = Wire->RouteNodes](int32 Begin, int32 End, EWireNodeType Type)
	{
		for (int32 I = Begin; I < End; ++I)
		{
			Test.TestEqual(TEXT("NodeType"), Nodes[I].NodeType, Type);
		}
	};

	// The original route nodes created by the Python script were lost during the wire
	// initialization right before the scene was stored to the AGX Dynamics archive. We can only see
	// the initialized nodes.
	RangeHasType(0, 11, EWireNodeType::Free);
	RangeHasType(11, 12, EWireNodeType::Eye);
	RangeHasType(12, 22, EWireNodeType::Free);
	RangeHasType(22, 23, EWireNodeType::BodyFixed);

	UAGX_RigidBodyComponent* WinchBodyRef = Winch.BodyAttachment.GetRigidBody();
	Test.TestEqual(TEXT("Winch Body"), WinchBodyRef, WinchBody);

	auto RangeHasBody =
		[this, &Nodes = Wire->RouteNodes](int32 Begin, int32 End, UAGX_RigidBodyComponent* Body)
	{
		for (int32 I = Begin; I < End; ++I)
		{
			UAGX_RigidBodyComponent* NodeBodyRef = Nodes[I].RigidBody.GetRigidBody();
			Test.TestEqual(TEXT("NodeBody"), NodeBodyRef, Body);
		}
	};

	RangeHasBody(0, 11, nullptr);
	RangeHasBody(11, 12, EyeBody);
	RangeHasBody(12, 22, nullptr);
	RangeHasBody(22, 23, BeadBody);

	return true;
}

/**
 * Remove eveything created by the archive import.
 *
 * @return True when the clearing is complete. Never returns false.
 */
bool FClearWireImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles {
		TEXT("ShapeMaterial"), TEXT("defaultWireMaterial_57.uasset")};
	AgxAutomationCommon::DeleteImportDirectory(TEXT("wire_build"), ExpectedFiles);

	return true;
}
#endif

//
// Constraint Dynamic Parameters test starts here.
//

class FImporterToSingleActor_ConstraintDynamicParametersTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckConstraintDynamicParametersImportedCommand,
	FImporterToSingleActor_ConstraintDynamicParametersTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearConstraintDynamicParametersImportedCommand,
	FImporterToSingleActor_ConstraintDynamicParametersTest&, Test);

class FImporterToSingleActor_ConstraintDynamicParametersTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_ConstraintDynamicParametersTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_ConstraintDynamicParametersTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.ConstraintDynamicParameters"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportArchiveSingleActorCommand(
			TEXT("constraint_dynamic_parameters_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckConstraintDynamicParametersImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearConstraintDynamicParametersImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_ConstraintDynamicParametersTest
		ImporterToSingleActor_ConstraintDynamicParametersTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script constraint_dynamic_parameters.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckConstraintDynamicParametersImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(
			TEXT("Could not import ConstraintDynamicParameters test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// Two Rigid Bodies, one Hinge constraint with two DofGraphicsComponent's and one
	// DofGraphicsComponent and one Default Scene Root.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 7);

	UAGX_ConstraintComponent* Constraint =
		GetByName<UAGX_ConstraintComponent>(Components, TEXT("constraint"));

	// Compliance.
	Test.TestEqual(
		TEXT("Translational 1 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational1), 100.0);
	Test.TestEqual(
		TEXT("Translational 2 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational2), 101.0);
	Test.TestEqual(
		TEXT("Translational 3 compliance"),
		Constraint->GetCompliance(EGenericDofIndex::Translational3), 102.0);
	Test.TestEqual(
		TEXT("Rotational 1 compliance"), Constraint->GetCompliance(EGenericDofIndex::Rotational1),
		103.0);
	Test.TestEqual(
		TEXT("Rotational 2 compliance"), Constraint->GetCompliance(EGenericDofIndex::Rotational2),
		104.0);
	// Rotational 3 is not supported for AGX::Hinge.

	// SpookDamping.
	Test.TestEqual(
		TEXT("Translational 1 damping"), Constraint->SpookDamping.Translational_1, 200.0);
	Test.TestEqual(
		TEXT("Translational 2 damping"), Constraint->SpookDamping.Translational_2, 201.0);
	Test.TestEqual(
		TEXT("Translational 3 damping"), Constraint->SpookDamping.Translational_3, 202.0);
	Test.TestEqual(TEXT("Rotational 1 damping"), Constraint->SpookDamping.Rotational_1, 203.0);
	Test.TestEqual(TEXT("Rotational 2 damping"), Constraint->SpookDamping.Rotational_2, 204.0);
	// Rotational 3 is not supported for AGX::Hinge.

	// Force range.
	Test.TestEqual(
		TEXT("Translational 1 force range min"), Constraint->ForceRange.Translational_1.Min, 300.f);
	Test.TestEqual(
		TEXT("Translational 1 force range max"), Constraint->ForceRange.Translational_1.Max, 301.f);
	Test.TestEqual(
		TEXT("Translational 2 force range min"), Constraint->ForceRange.Translational_2.Min, 302.f);
	Test.TestEqual(
		TEXT("Translational 2 force range max"), Constraint->ForceRange.Translational_2.Max, 303.f);
	Test.TestEqual(
		TEXT("Translational 3 force range min"), Constraint->ForceRange.Translational_3.Min, 304.f);
	Test.TestEqual(
		TEXT("Translational 3 force range max"), Constraint->ForceRange.Translational_3.Max, 305.f);
	Test.TestEqual(
		TEXT("Rotational 1 force range min"), Constraint->ForceRange.Rotational_1.Min, 306.f);
	Test.TestEqual(
		TEXT("Rotational 1 force range min"), Constraint->ForceRange.Rotational_1.Max, 307.f);
	Test.TestEqual(
		TEXT("Rotational 2 force range min"), Constraint->ForceRange.Rotational_2.Min, 308.f);
	Test.TestEqual(
		TEXT("Rotational 2 force range min"), Constraint->ForceRange.Rotational_2.Max, 309.f);
	// Rotational 3 is not supported for AGX::Hinge.

	Test.TestEqual(TEXT("Solve type"), Constraint->SolveType, EAGX_SolveType::StDirectAndIterative);
	Test.TestEqual(TEXT("Enabled"), Constraint->bEnable, true);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearConstraintDynamicParametersImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

	return true;
}

//
// Rigid Body properties test starts here.
//

class FImporterToSingleActor_RigidBodyPropertiesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRigidBodyPropertiesImportedCommand, FImporterToSingleActor_RigidBodyPropertiesTest&,
	Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRigidBodyPropertiesImportedCommand, FImporterToSingleActor_RigidBodyPropertiesTest&,
	Test);

class FImporterToSingleActor_RigidBodyPropertiesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_RigidBodyPropertiesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_RigidBodyPropertiesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.RigidBodyProperties"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportArchiveSingleActorCommand(
			TEXT("rigidbody_properties_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckRigidBodyPropertiesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearRigidBodyPropertiesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_RigidBodyPropertiesTest ImporterToSingleActor_RigidBodyPropertiesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script rigidbody_properties.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckRigidBodyPropertiesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import RigidBodyProperties test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// One Rigid Bodies, one Geometry and one Default Scene Root.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 3);

	UAGX_RigidBodyComponent* SphereBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("SphereBody"));

	// Name.
	{
		Test.TestEqual("Sphere name", SphereBody->GetFName(), FName(TEXT("SphereBody")));
	}

	// Position.
	{
		FVector Actual = SphereBody->GetComponentLocation();
		// The position, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(10.f, 20.f, 30.f);
		FVector Expected = AgxToUnrealVector(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere position"), Actual, Expected);
	}

	// Rotation.
	{
		FRotator Actual = SphereBody->GetComponentRotation();
		// The rotation, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(0.1f, 0.2f, 0.3f);
		FRotator Expected = AgxToUnrealEulerAngles(ExpectedAgx);
		TestEqual(Test, TEXT("Sphere rotation"), Actual, Expected);
	}

	// Velocity.
	{
		FVector Actual = SphereBody->Velocity;
		// The velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(1.f, 2.f, 3.f);
		FVector Expected = AgxToUnrealVector(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere linear velocity"), Actual, Expected);
	}

	// Angular velocity.
	{
		FVector Actual = SphereBody->AngularVelocity;
		// The angular velocity, in AGX Dynamics' units, that was given to the sphere when created.
		FVector ExpectedAgx(1.1f, 1.2f, 1.3f);
		FVector Expected = AgxToUnrealAngularVelocity(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere angular velocity"), Actual, Expected);
	}

	// Mass.
	{
		Test.TestEqual(TEXT("Sphere mass"), SphereBody->Mass, 500.f);
	}

	// Mass properties automatic generation.
	{
		Test.TestEqual(TEXT("Auto generate mass"), SphereBody->GetAutoGenerateMass(), false);
		Test.TestEqual(
			TEXT("Auto generate CoM offset"), SphereBody->GetAutoGenerateCenterOfMassOffset(),
			true);
		Test.TestEqual(
			TEXT("Auto generate inertia"), SphereBody->GetAutoGeneratePrincipalInertia(), false);
	}

	// Inertia tensor diagonal.
	{
		FVector Actual = SphereBody->GetPrincipalInertia();
		FVector Expected(100.f, 200.f, 300.f);
		Test.TestEqual(TEXT("Sphere inertia tensor diagonal"), Actual, Expected);
	}

	// Motion control.
	{
		EAGX_MotionControl Actual = SphereBody->MotionControl;
		EAGX_MotionControl Expected = EAGX_MotionControl::MC_DYNAMICS;
		Test.TestEqual(TEXT("Sphere motion control"), Actual, Expected);
	}

	// Transform root component.
	{
		Test.TestEqual(
			TEXT("Sphere transform target"), SphereBody->TransformTarget,
			EAGX_TransformTarget::TT_SELF);
	}

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearRigidBodyPropertiesImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

	return true;
}

//
// Simple geometries test starts here.
//

class FImporterToSingleActor_SimpleGeometriesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSimpleGeometriesImportedCommand, FImporterToSingleActor_SimpleGeometriesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSimpleGeometriesImportedCommand, FImporterToSingleActor_SimpleGeometriesTest&, Test);

class FImporterToSingleActor_SimpleGeometriesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_SimpleGeometriesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_SimpleGeometriesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.SimpleGeometries"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.
	UAGX_RigidBodyComponent* TrimeshBody = nullptr;

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("single_geometries_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSimpleGeometriesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearSimpleGeometriesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_SimpleGeometriesTest ImporterToSingleActor_SimpleGeometriesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script single_geometries.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckSimpleGeometriesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import SimpleGeometries test scene: No content created."));
		return true;
	}

	auto testShape = [this](USceneComponent* c, const FVector& ExpectedAGXWorldPos)
	{
		Test.TestNotNull(TEXT("Component exists"), c);
		const FVector ExpectedUnrealPos = AgxToUnrealVector(ExpectedAGXWorldPos);
		Test.TestEqual(TEXT("Component position"), c->GetComponentLocation(), ExpectedUnrealPos);
	};

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// 5 Rigid Bodies, 10 Geometries, 2 Static Meshes and 1 Default Scene Root.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 18);

	testShape(
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("sphereGeometry")),
		FVector(0.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_BoxShapeComponent>(Components, TEXT("boxGeometry")), FVector(2.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_CylinderShapeComponent>(Components, TEXT("cylinderGeometry")),
		FVector(4.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_CapsuleShapeComponent>(Components, TEXT("capsuleGeometry")),
		FVector(6.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("trimeshGeometry")),
		FVector(8.f, 0.f, 0.f));

	testShape(
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("sphereGeometryFree")),
		FVector(0.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_BoxShapeComponent>(Components, TEXT("boxGeometryFree")),
		FVector(2.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_CylinderShapeComponent>(Components, TEXT("cylinderGeometryFree")),
		FVector(4.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_CapsuleShapeComponent>(Components, TEXT("capsuleGeometryFree")),
		FVector(6.f, 2.f, 0.f));

	testShape(
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("trimeshGeometryFree")),
		FVector(8.f, 2.f, 0.f));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearSimpleGeometriesImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {
		TEXT("StaticMesh"), TEXT("trimeshShape.uasset"), TEXT("trimeshShapeFree.uasset")};
	AgxAutomationCommon::DeleteImportDirectory(TEXT("single_geometries_build"), ExpectedFiles);

	return true;
}

//
// Contact materials test starts here.
//

class FArchiveImporterToSingleActor_ContactMaterialsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckContactMaterialsImportedCommand, FArchiveImporterToSingleActor_ContactMaterialsTest&,
	Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearContactMaterialsImportedCommand, FArchiveImporterToSingleActor_ContactMaterialsTest&,
	Test);

class FArchiveImporterToSingleActor_ContactMaterialsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToSingleActor_ContactMaterialsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToSingleActor_ContactMaterialsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.ContactMaterials"))
	{
	}

public:
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand(TEXT("contact_materials_build.agx"), Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckContactMaterialsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearContactMaterialsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FArchiveImporterToSingleActor_ContactMaterialsTest
		ArchiveImporterToSingleActor_ContactMaterialsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script contact_materials.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckContactMaterialsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import ContactMaterials test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// 4 Rigid Bodies, 4 Geometries, 1 Contact Material Registrar and 1 Default Scene Root.
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 10);

	UAGX_ContactMaterialRegistrarComponent* Registrar =
		GetByName<UAGX_ContactMaterialRegistrarComponent>(
			Components, TEXT("AGX_ContactMaterialRegistrar"));

	Test.TestNotNull("Contact Material Registrar", Registrar);
	Test.TestEqual("Num Contact Materials in Registrar", Registrar->ContactMaterials.Num(), 2);

	UAGX_ContactMaterialBase** Cm1 = Registrar->ContactMaterials.FindByPredicate(
		[](UAGX_ContactMaterialBase* Cm) { return Cm->GetName() == "CMMat1Mat2"; });
	Test.TestNotNull("Cm1", Cm1);
	Test.TestNotNull("Cm1 Material1", (*Cm1)->Material1);
	Test.TestNotNull("Cm1 Material2", (*Cm1)->Material2);
	Test.TestEqual("Cm1 Material1 name", (*Cm1)->Material1->GetName(), "Mat1");
	Test.TestEqual("Cm1 Material2 name", (*Cm1)->Material2->GetName(), "Mat2");
	Test.TestEqual("Cm1 contact solver", (*Cm1)->ContactSolver, EAGX_ContactSolver::Iterative);
	Test.TestEqual(
		"Cm1 friction model", (*Cm1)->FrictionModel,
		EAGX_FrictionModel::IterativeProjectedConeFriction);
	Test.TestEqual(
		"Cm1 contact reduction bin resolution", (*Cm1)->ContactReduction.BinResolution, 2);
	Test.TestEqual("Cm1 friction coefficient", (*Cm1)->FrictionCoefficient, 0.11);
	Test.TestEqual("Cm1 surface viscosity", (*Cm1)->SurfaceViscosity, 0.12);
	Test.TestEqual("Cm1 restitution", (*Cm1)->Restitution, 0.13);
	Test.TestEqual("Cm1 youngs modulus", (*Cm1)->YoungsModulus, 123451234.0);
	Test.TestEqual("Cm1 damping", (*Cm1)->SpookDamping, 0.14);
	Test.TestEqual("Cm1 adhesive force", (*Cm1)->AdhesiveForce, 0.15);
	Test.TestEqual(
		"Cm1 adhesive overlap", (float) (*Cm1)->AdhesiveOverlap, AgxToUnrealDistance(0.16));

	UAGX_ContactMaterialBase** Cm2 = Registrar->ContactMaterials.FindByPredicate(
		[](UAGX_ContactMaterialBase* Cm) { return Cm->GetName() == "CMMat3Mat4"; });
	Test.TestNotNull("Cm2", Cm2);
	Test.TestNotNull("Cm2 Material1", (*Cm2)->Material1);
	Test.TestNotNull("Cm2 Material2", (*Cm2)->Material2);
	Test.TestEqual("Cm2 Material1 name", (*Cm2)->Material1->GetName(), "Mat3");
	Test.TestEqual("Cm2 Material2 name", (*Cm2)->Material2->GetName(), "Mat4");
	Test.TestEqual("Cm2 contact solver", (*Cm2)->ContactSolver, EAGX_ContactSolver::Direct);
	Test.TestEqual("Cm2 friction model", (*Cm2)->FrictionModel, EAGX_FrictionModel::BoxFriction);
	Test.TestEqual(
		"Cm2 contact reduction bin resolution", (*Cm2)->ContactReduction.BinResolution, 3);
	Test.TestEqual("Cm2 friction coefficient", (*Cm2)->FrictionCoefficient, 0.21);
	Test.TestEqual("Cm2 surface viscosity", (*Cm2)->SurfaceViscosity, 0.22);
	Test.TestEqual("Cm2 restitution", (*Cm2)->Restitution, 0.23);
	Test.TestEqual("Cm2 youngs modulus", (*Cm2)->YoungsModulus, 101010101.0);
	Test.TestEqual("Cm2 damping", (*Cm2)->SpookDamping, 0.24);
	Test.TestEqual("Cm2 adhesive force", (*Cm2)->AdhesiveForce, 0.25);
	Test.TestEqual(
		"Cm2 adhesive overlap", (float) (*Cm2)->AdhesiveOverlap, AgxToUnrealDistance(0.26));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearContactMaterialsImportedCommand::Update()
{
	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	TArray<const TCHAR*> ExpectedFiles = {TEXT("ContactMaterial"),	 TEXT("CMMat1Mat2.uasset"),
										  TEXT("CMMat3Mat4.uasset"), TEXT("ShapeMaterial"),
										  TEXT("Mat1.uasset"),		 TEXT("Mat2.uasset"),
										  TEXT("Mat3.uasset"),		 TEXT("Mat4.uasset")};
	AgxAutomationCommon::DeleteImportDirectory(TEXT("contact_materials_build"), ExpectedFiles);

	return true;
}

//
// URDF link with meshes test starts here.
//

class FImporterToSingleActor_URDFLinkWithMeshesTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinkWithMeshesImportedCommand, FImporterToSingleActor_URDFLinkWithMeshesTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinkWithMeshesImportedCommand, FImporterToSingleActor_URDFLinkWithMeshesTest&, Test);

class FImporterToSingleActor_URDFLinkWithMeshesTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_URDFLinkWithMeshesTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_URDFLinkWithMeshesTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.URDFLinkWithMeshes"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFSingleActorCommand(
			TEXT("link_with_meshes.urdf"), AgxAutomationCommon::GetTestSceneDirPath(), Contents,
			*this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinkWithMeshesImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinkWithMeshesImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_URDFLinkWithMeshesTest ImporterToSingleActor_URDFLinkWithMeshesTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script link_with_meshes.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT("Could not import URDFLinkWithMeshes test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// One DefaultSceneRoot, one Rigid Body, one Trimesh with a render mesh and a collision mesh
	// and one Trimesh with only one collision mesh.
	Test.TestEqual("Number of components", Components.Num(), 7);

	UAGX_TrimeshShapeComponent* Urdfmeshvisual =
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("urdfmeshvisual"));
	UAGX_TrimeshShapeComponent* Urdfmeshcollision =
		GetByName<UAGX_TrimeshShapeComponent>(Components, TEXT("urdfmeshcollision"));

	Test.TestNotNull(TEXT("Urdfmeshvisual"), Urdfmeshvisual);
	Test.TestNotNull(TEXT("Urdfmeshcollision"), Urdfmeshcollision);

	Test.TestFalse("Urdfmeshvisual collide", Urdfmeshvisual->bCanCollide);
	Test.TestTrue("Urdfmeshcollision collide", Urdfmeshcollision->bCanCollide);

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinkWithMeshesImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.Contents == nullptr)
	{
		return true;
	}

	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	TArray<FString> Assets = GetReferencedStaticMeshAssets(Components);
	if (Assets.Num() != 3)
	{
		Test.AddError(TEXT("Unexpected number of assets found."));
		return true;
	}

	TArray<const TCHAR*> FilesAndDirsToRemove;
	FilesAndDirsToRemove.Add(TEXT("RenderMesh"));
	FilesAndDirsToRemove.Add(TEXT("StaticMesh"));
	for (const FString& Asset : Assets)
	{
		FilesAndDirsToRemove.Add(*Asset);
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

#if defined(__linux__)
	/// @todo Workaround for internal issue #213.
	Test.AddExpectedError(
		TEXT("inotify_rm_watch cannot remove descriptor"), EAutomationExpectedErrorFlags::Contains,
		0);
	Test.AddError(TEXT("inotify_rm_watch cannot remove descriptor"));
#endif

	AgxAutomationCommon::DeleteImportDirectory(TEXT("link_with_meshes"), FilesAndDirsToRemove);

	return true;
}

//
// URDF links geometries constraints test starts here.
//

class FImporterToSingleActor_URDFLinksGeometriesConstraintsTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToSingleActor_URDFLinksGeometriesConstraintsTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearURDFLinksGeometriesConstraintsImportedCommand,
	FImporterToSingleActor_URDFLinksGeometriesConstraintsTest&, Test);

class FImporterToSingleActor_URDFLinksGeometriesConstraintsTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FImporterToSingleActor_URDFLinksGeometriesConstraintsTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FImporterToSingleActor_URDFLinksGeometriesConstraintsTest"),
			  TEXT("AGXUnreal.Editor.ImporterToSingleActor.URDFLinksGeometriesConstraints"))
	{
	}

public:
	UWorld* World = nullptr;
	UAGX_Simulation* Simulation = nullptr;
	AActor* Contents = nullptr; /// <! The Actor created to hold the archive contents.

protected:
	virtual bool RunTest(const FString&) override
	{
		BAIL_TEST_IF_NOT_EDITOR(false)
		ADD_LATENT_AUTOMATION_COMMAND(FImportURDFSingleActorCommand(
			TEXT("links_geometries_constraints.urdf"), "", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckURDFLinksGeometriesConstraintsImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FClearURDFLinksGeometriesConstraintsImportedCommand(*this))
		return true;
	}
};

namespace
{
	FImporterToSingleActor_URDFLinksGeometriesConstraintsTest
		ImporterToSingleActor_URDFLinksGeometriesConstraintsTest;
}

/**
 * Check that the expected state was created during import.
 *
 * The object structure and all numbers tested here should match what is being set in the source
 * script links_geometries_constraints.agxPy.
 * @return true when the check is complete. Never returns false.
 */
bool FCheckURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;
	if (Test.Contents == nullptr)
	{
		Test.AddError(TEXT(
			"Could not import URDFLinksGeometriesConstraints test scene: No content created."));
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);

	// 1 DefaultSceneRoot, 4 Rigid Bodies, 4 Shape Components and 2 Constraints with 3 Graphics
	// components each.
	Test.TestEqual("Number of components", Components.Num(), 17);

	UAGX_RigidBodyComponent* Boxlink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("boxlink"));
	UAGX_RigidBodyComponent* Shperelink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("spherelink"));
	UAGX_RigidBodyComponent* Cylinderlink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("cylinderlink"));
	UAGX_RigidBodyComponent* Freefallinglink =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("freefallinglink"));

	Test.TestNotNull(TEXT("Boxlink"), Boxlink);
	Test.TestNotNull(TEXT("Shperelink"), Shperelink);
	Test.TestNotNull(TEXT("Cylinderlink"), Cylinderlink);
	Test.TestNotNull(TEXT("Freefallinglink"), Freefallinglink);

	if (Boxlink == nullptr || Shperelink == nullptr || Cylinderlink == nullptr ||
		Freefallinglink == nullptr)
	{
		Test.AddError("At least one Rigid Body was nullptr, cannot continue.");
		return true;
	}

	Test.TestEqual(
		TEXT("Boxlink position"), Boxlink->GetComponentLocation(),
		AgxToUnrealVector({0.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Shperelink position"), Shperelink->GetComponentLocation(),
		AgxToUnrealVector({1.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Cylinderlink position"), Cylinderlink->GetComponentLocation(),
		AgxToUnrealVector({2.f, 0.f, 0.f}));

	Test.TestEqual(
		TEXT("Freefallinglink position"), Freefallinglink->GetComponentLocation(),
		AgxToUnrealVector({0.f, 0.f, 0.f}));

	return true;
}

/**
 * Remove everything created by the archive import.
 * @return true when the clearing is complete. Never returns false.
 */
bool FClearURDFLinksGeometriesConstraintsImportedCommand::Update()
{
	using namespace AgxAutomationCommon;

	if (Test.Contents == nullptr)
	{
		return true;
	}

	UWorld* World = Test.Contents->GetWorld();
	if (World != nullptr)
	{
		World->DestroyActor(Test.Contents);
	}

	return true;
}

// WITH_DEV_AUTOMATION_TESTS
#endif
