
// AGXUnreal includes.
#include "AGX_ArchiveImporterToSingleActor.h"
#include "AGX_EditorUtilities.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "AgxAutomationCommon.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

/// @todo Add the `#if WITH_DEV_AUTOMATION_TESTS` macro before the function definition

/*
 * This file contains a set of tests that deal with importing AGX Dynamics archives into an Actor
 * with ActorComponents for each imported AGX Dynamics body etc. All importing is done using
 * AGX_ArchiveImporterToSingleActor::ImportAGXArchive.
 */

/**
 * Latent Command that imports an AGX Dynamics archive into a single actor.
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
 */
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckEmptySceneImportedCommand, AActor*&, Contents, FAutomationTestBase&, Test);
bool FCheckEmptySceneImportedCommand::Update()
{
	// The Actor's only component should be the root component.
	TArray<UActorComponent*> Components;
	Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 1);
	USceneComponent* SceneRoot =
		AgxAutomationCommon::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);

	// The Actor should have been created in the test world.
	UWorld* World = AgxAutomationCommon::GetTestWorld();
	Test.TestEqual(TEXT("The actor should be in the test world."), Contents->GetWorld(), World);
	Test.TestTrue(TEXT("The actor should be in the test world."), World->ContainsActor(Contents));

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
		/// each test may be required. I really hope multiple tests don't run concurrently on the
		/// same world.
#if 0
		// ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")));
		// ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("empty_scene.agx", Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckEmptySceneImportedCommand(Contents, *this));
		return true;
	}

private:
	AActor* Contents;
};

namespace
{
	FArchiveImporterToSingleActor_EmptySceneTest ArchiveImporterToSingleActor_EmptySceneTest;
}


#if 0
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArchiveImporterToSingleActor_SingleSphereTest,
	"AGXUnreal.ArchiveImporterToSingleActor.SingleSphere",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
#endif

class FArchiveImporterToSingleActor_SingleSphereTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSingleSphereImportedCommand, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSphereHasMoved, FArchiveImporterToSingleActor_SingleSphereTest&, Test);

/**
 * Latent Command that waits for the given time, starting at the time when the Latent Command is
 * first updated.
 */
class FWaitUntilTimeRelative final : public IAutomationLatentCommand
{
public:
	FWaitUntilTimeRelative(
		float InTimeToWait, FArchiveImporterToSingleActor_SingleSphereTest& InTest)
		: Test(InTest)
		, TimeToWait(InTimeToWait)
	{
	}

	virtual bool Update();

private:
	void Init();
	bool HaveReachedEndTime();
	void Shutdown();

private:
	FArchiveImporterToSingleActor_SingleSphereTest& Test;
	float TimeToWait;
	float EndTime;
	int32 NumUpdates = 0;
};

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
		Simulation = UAGX_Simulation::GetFrom(World);

		// See comment in FArchiveImporterToSingleActor_EmptySceneTest.
		// In short, loading a map stops world ticking.
#if 0
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
#endif

		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("single_sphere.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSingleSphereImportedCommand(*this))

		/// @todo Using local game time waiter until we know it works as intended.
		// ADD_LATENT_AUTOMATION_COMMAND(FTickUntilCommand(World, 1.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilTimeRelative(1.0f, *this));
		// ADD_LATENT_AUTOMATION_COMMAND(FTickForDuration(World, 1.0f, 1.0f / 60.0f, *this));

		ADD_LATENT_AUTOMATION_COMMAND(FCheckSphereHasMoved(*this));

		return true;
	}
};

namespace
{
	FArchiveImporterToSingleActor_SingleSphereTest ArchiveImporterToSingleActor_SingleSphereTest;
}

bool FCheckSingleSphereImportedCommand::Update()
{
	if (Test.World == nullptr)
	{
		return true;
	}

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 3);

	// Get the components we know should be there.
	USceneComponent* SceneRoot =
		AgxAutomationCommon::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* BulletBody =
		AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
	UAGX_SphereShapeComponent* BulletShape =
		AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(Components, TEXT("bullet_1"));

	// Make sure we got the components we know should be there.
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	Test.TestNotNull(TEXT("Bullet"), BulletBody);
	Test.TestNotNull(TEXT("Bullet_1"), BulletShape);

	// Read and verify state for each UAGX_RigidBodyComponent property.
	float Mass = BulletBody->Mass;
	Test.TestEqual(TEXT("Sphere mass"), Mass, 100.0f);

	{
		FVector LinearVelocity = BulletBody->Velocity;
		// The velocity, in AGX Dynamics' units, that was given to the sphere when created.
		/// @todo Replace these numbers once we get a dedicated test scene.
		FVector AgxVelocity(-4.73094, 16.5768, 10.9014);
		FVector AgxToUnreal(100.0f, -100.0f, 100.0f);
		FVector ExpectedVelocity = AgxVelocity * AgxToUnreal;
		Test.TestEqual(TEXT("Sphere linear velocity"), LinearVelocity, ExpectedVelocity, 1e-2);
	}

	{
		FVector ActualAngularVelocity = BulletBody->AngularVelocity;
		// The angular velocity, in AGX Dynamics' units, that was given to the sphere when created.
		/// @todo Replace these numbers once we get a dedicated test scene.
		FVector AgxAngularVelocity(17.7668f, 2.27498f, 7.87081f);
		FVector AgxToUnreal(1.0f, -1.0f, -1.0f);
		FVector ExpectedAngularVelocity = AgxAngularVelocity * AgxToUnreal;
		Test.TestEqual(
			TEXT("Sphere angular velocity"), ActualAngularVelocity, ExpectedAngularVelocity);
	}

	EAGX_MotionControl MotionControl = BulletBody->MotionControl;
	Test.TestEqual(TEXT("Sphere motion control"), MotionControl, EAGX_MotionControl::MC_DYNAMICS);

	uint8_t bTransformRootComponent = BulletBody->bTransformRootComponent;
	Test.TestFalse(TEXT("Sphere transform root component"), bTransformRootComponent);

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
	bool bHasNative = BulletBody->HasNative();
	Test.TestTrue(TEXT("Sphere has native"), bHasNative);

	UWorld* BodyWorld = BulletBody->GetWorld();
	Test.TestEqual(TEXT("Sphere world"), BodyWorld, Test.World);

#if 0
	UE_LOG(
			LogAGX, Warning, TEXT("Body has velocity (%f, %f, %f)."), LinearVelocity.X,
			LinearVelocity.Y, LinearVelocity.Z);
#endif

	// Publish the important bits to the rest of the test.
	Test.SphereBody = BulletBody;
	Test.StartPosition = BulletBody->GetComponentLocation();
	Test.StartVelocity = BulletBody->Velocity;

	return true;
}

bool FWaitUntilTimeRelative::Update()
{
	if (Test.World == nullptr)
	{
		return true;
	}
	if (NumUpdates == 0)
	{
		Init();
	}

#if 0
	UE_LOG(
		LogAGX, Warning, TEXT("Update number %d with World time %f and Simulation time %f."),
		NumUpdates, World->GetTimeSeconds(), UAGX_Simulation::GetFrom(World)->GetTimeStamp());
#endif

	++NumUpdates;

	/// @todo Safety bail-out to prevent test hangs. Decide if really needed and remove if not.
	if (NumUpdates > 1000)
	{
		Test.AddError("Did not reach the time event after many ticks. Giving up.");
		return true;
	}

	if (HaveReachedEndTime())
	{
		Shutdown();
		return true;
	}
	else
	{
		return false;
	}
}

void FWaitUntilTimeRelative::Init()
{
	Test.StartUnrealTime = Test.World->GetTimeSeconds();
	EndTime = Test.StartUnrealTime + TimeToWait;
	UAGX_Simulation::GetFrom(Test.World)->SetTimeStamp(StartTime);
	Test.StartAgxTime = UAGX_Simulation::GetFrom(Test.World)->GetTimeStamp();

	UE_LOG(
		LogAGX, Warning,
		TEXT("Waiting for %f seconds, starting at world time %f and simulation time %f."),
		TimeToWait, Test.World->GetTimeSeconds(),
		UAGX_Simulation::GetFrom(Test.World)->GetTimeStamp());
}

bool FWaitUntilTimeRelative::HaveReachedEndTime()
{
	return Test.World->GetTimeSeconds() >= EndTime;
}

void FWaitUntilTimeRelative::Shutdown()
{
	Test.EndUnrealTime = Test.World->GetTimeSeconds();
	Test.EndAgxTime = UAGX_Simulation::GetFrom(Test.World)->GetTimeStamp();
}

float RelativeTolerance(float Expected, float Tolerance)
{
	return FMath::Abs(Expected * Tolerance);
}

bool FCheckSphereHasMoved::Update()
{
	if (Test.SphereBody == nullptr)
	{
		return true;
	}

	// Test.TestEqual("World and AGX times should be equal", Test.StartAgxTime,
	// Test.StartUnrealTime);

	FVector EndPosition = Test.SphereBody->GetComponentLocation();
	FVector EndVelocity = Test.SphereBody->Velocity;

	UE_LOG(
		LogAGX, Warning, TEXT("Sphere positions:\n   %s\n   %s"), *Test.StartPosition.ToString(),
		*EndPosition.ToString())

	UE_LOG(
		LogAGX, Warning, TEXT("Sphere velocities:\n   %s\n   %s"), *Test.StartVelocity.ToString(),
		*EndVelocity.ToString())



	float Duration = Test.EndAgxTime - Test.StartAgxTime;
	UE_LOG(LogAGX, Warning, TEXT("Simulated for %f seconds."), Duration);

	// Velocity constant only for X and Y directions. Z has gravity.
	for (int32 I : {0, 1})
	{
		float ExpectedDisplacement = EndVelocity[I] * Duration;
		float ExpectedPosition = Test.StartPosition[I] + ExpectedDisplacement;
		float ActualPosition = EndPosition[I];
		Test.TestEqual(
			"Body should move according to velocity.", ActualPosition, ExpectedPosition,
			RelativeTolerance(ExpectedPosition, 0.01f));
	}

	// Velocity test for Z.
	{
		float StartVelocity = Test.StartVelocity.Z;
		float StartPosition = Test.StartPosition.Z;
		float Acceleration = UAGX_Simulation::GetFrom(Test.World)->Gravity.Z;
		float ExpectedPosition =
			StartPosition + StartVelocity * Duration + 0.5f * Acceleration * Duration * Duration;
		float ActualPosition = EndPosition.Z;
		Test.TestEqual(
			"Velocity in the Z direction should be subject to gravity.", ActualPosition,
			ExpectedPosition, RelativeTolerance(ExpectedPosition, 0.02f));
	}

	// FVector EndVelocity = Test.SphereBody->Velocity;
	// Test.TestEqual("Sphere should accelerate due to gravity", EndVelocity, Test.StartVelocity);
	return true;
}
