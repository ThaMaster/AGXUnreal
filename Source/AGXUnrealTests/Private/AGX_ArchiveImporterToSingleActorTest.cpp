
// AGXUnreal includes.
#include "AGX_ArchiveImporterToSingleActor.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

/*
 * This file contains a set of tests for AGX_ArchiveImporterToSingleActor, which imports an AGX
 * Dynamics archive into the current world as a single Actor that contains ActorComponents for each
 * imported object.
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
	Test.TestEqual(
		TEXT("TestWorld and CurrentWorld"), AgxAutomationCommon::GetTestWorld(),
		FAGX_EditorUtilities::GetCurrentWorld());
	FString ArchiveFilePath = AgxAutomationCommon::GetArchivePath(ArchiveName);
	Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
	Test.TestNotNull(TEXT("Contents"), Contents);
	return true;
}

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
 * removal isn't done immediately by Unreal Engine so it may be neccessary to do an extra tick after
 * this Latent Command to let the change finalize.
 * @todo Write this Latent command so that it returns false on the first call to Update and true on
 * the second.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FClearEmptySceneImportedCommand, AActor*&, Contents);
bool FClearEmptySceneImportedCommand::Update()
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

/**
 * Test that an empty AGX Dynamics archive can be imported, that the archive Actor root is created
 * as it should, and that it is added to the world.
 */
class FArchiveImporterToSingleActor_EmptySceneTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToSingleActor_EmptySceneTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToSingleActor_EmptySceneTest"),
			  TEXT("AGXUnreal.ArchiveImporterToSingleActor.EmptyScene.Test"))
	{
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		BAIL_TEST_IF_NO_WORLD()
		BAIL_TEST_IF_WORLDS_MISMATCH()

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
	FArchiveImporterToSingleActor_EmptySceneTest ArchiveImporterToSingleActor_EmptySceneTest;
}

class FArchiveImporterToSingleActor_SingleSphereTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSingleSphereImportedCommand, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreInitialTimes, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FStoreResultingTimes, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSphereHasMoved, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSingleSphereImportedCommand, AActor*&, Contents);

class FArchiveImporterToSingleActor_SingleSphereTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToSingleActor_SingleSphereTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToSingleActor_SingleSphereTest"),
			  TEXT("AGXUnreal.ArchiveImporterToSingleActor.SingleSphere.Test"))
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
		BAIL_TEST_IF_CANT_SIMULATE()
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

		// See comment in FArchiveImporterToSingleActor_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("single_sphere_build.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSingleSphereImportedCommand(*this))
		ADD_LATENT_AUTOMATION_COMMAND(FStoreInitialTimes(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitWorldDuration(World, 1.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FStoreResultingTimes(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSphereHasMoved(*this));
		ADD_LATENT_AUTOMATION_COMMAND(FClearSingleSphereImportedCommand(Contents));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitNTicks(1));

		return true;
	}
};

namespace
{
	FArchiveImporterToSingleActor_SingleSphereTest ArchiveImporterToSingleActor_SingleSphereTest;
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
		FVector Actual = SphereBody->InertiaTensorDiagonal;
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
		Test.TestFalse(
			TEXT("Sphere transform root component"), SphereBody->bTransformRootComponent);
	}

	// Radius.
	{
		float Actual = SphereShape->Radius;
		float ExpectedAgx = 5.00000000000000000000e-01f;
		float Expected = AgxToUnrealDistance(ExpectedAgx);
		Test.TestEqual(TEXT("Sphere radius"), Actual, Expected);
	}

	/**
	 * @todo A native AGX Dynamics RigidBody is created for the body when the AGX_RigidBodyComponent
	 * is registered with the owning actor. This is because engine code detects that the Actors has
	 * a World already, the editor world that the ArchiveImported passed to the ArchiveReader, so
	 * BeginPlay is called on the AGX_RigidBodyComponent immediately. I'm not entirely sure that is
	 * what we want.
	 *
	 * The above is incorrect. It's not the Editor world but the Game world created because
	 * Automation Tests are run with -Game so a Game world is created that causes the BeginPlay to
	 * be called. Something causes BeginPlay to be called on the Actor very early, which in turn
	 * causes BeginPlay to be called on the AGX_RigidBody from UActorComponent::RegisterComponent.
	 * That's when the Native object is created.
	 */
	bool bHasNative = SphereBody->HasNative();
	Test.TestTrue(TEXT("Sphere has native"), bHasNative);

	UWorld* BodyWorld = SphereBody->GetWorld();
	Test.TestEqual(TEXT("Sphere world"), BodyWorld, Test.World);

#if 0
	UE_LOG(
			LogAGX, Warning, TEXT("Body has velocity (%f, %f, %f)."), LinearVelocity.X,
			LinearVelocity.Y, LinearVelocity.Z);
#endif

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
	Simulation->StepMode = SM_CATCH_UP_IMMEDIATELY;
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
		float Acceleration = UAGX_Simulation::GetFrom(Test.World)->Gravity.Z;
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

// WITH_DEV_AUTOMATION_TESTS
#endif
