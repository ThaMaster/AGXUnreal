
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

/*
 * This file contains a set of tests that deal with importing AGX Dynamics archives into an Actor
 * with ActorComponents for each imported AGX Dynamics body etc. All importing is done using
 * AGX_ArchiveImporterToSingleActor::ImportAGXArchive.
 */

/**
 * @todo Remove this test code.
 * The purpose of this test is to figure out how to abort a test from a Latent Command so that
 * the subsequent Latent Commands are skipped. I haven't found a way yet and I'm starting to suspect
 * that it would be a bad idea. Tests should clean up after themselves which must happen in a Latent
 * Command. By aborting the test we would also skip the clean-up step. I don't think we have any
 * flow control / exceptions / then-else support in Latent Commands. A better approach would be to
 * write each Latent Command to handle failures in the preceding Latent Commands.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTestAbort, "AGXUnreal.TestAbort",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter);
bool FTestAbort::RunTest(const FString& Parameters)
{
	using namespace AgxAutomationCommon;
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("Before the error")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogErrorAgxCommand(TEXT("At the error.")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("After the error.")));
	return true;
}

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
	UWorld* World = AgxAutomationCommon::GetTestWorld();
	Test.TestEqual(TEXT("The actor's world and the test world."), Contents->GetWorld(), World);

	TArray<UActorComponent*> Components;
	Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 1);
	USceneComponent* SceneRoot =
		AgxAutomationCommon::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	Test.TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);

	bool Found = false;
	for (FActorIterator It(World); It; ++It)
	{
		if (*It == Contents)
		{
			Found = true;
			break;
		}
	}
	Test.TestTrue(TEXT("Imported actor found in test world."), Found);

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
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
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

BEGIN_DEFINE_SPEC(
	FArchiveImporterToSingleActor_EmptySceneSpec,
	"AGXUnreal.ArchiveImporterToSingleActor.EmptyScene.Spec", AgxAutomationCommon::DefaultTestFlags)
UWorld* World = nullptr;
AActor* Contents = nullptr;
END_DEFINE_SPEC(FArchiveImporterToSingleActor_EmptySceneSpec)

void FArchiveImporterToSingleActor_EmptySceneSpec::Define()
{
	Describe("Import empty scene", [this]() {
		BeforeEach([this]() {
			World = AgxAutomationCommon::GetTestWorld();
			TestNotNull(TEXT("World"), World);
			UE_LOG(LogAGX, Warning, TEXT("Got world %p"), (void*) World);
		});

		BeforeEach([this]() {
			UE_LOG(LogAGX, Warning, TEXT("Opening map 'Test_ArchiveImport'."));
			GEngine->Exec(World, TEXT("open Test_ArchiveImport"));
		});

		BeforeEach([this]() {
			FString ArchiveName = TEXT("empty_scene.agx");
			FString ArchiveFilePath = AgxAutomationCommon::GetArchivePath(ArchiveName);
			Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
			UE_LOG(LogAGX, Warning, TEXT("Imported archive '%s'."), *ArchiveFilePath);
		});

		It("should contain the archive actor", [this]() {
			UE_LOG(LogAGX, Warning, TEXT("Running checks"));
			TestNotNull(TEXT(""), Contents);

			bool Found = false;
			for (FActorIterator It(World); It; ++It)
			{
				if (*It == Contents)
				{
					Found = true;
					break;
				}
			}
			TestTrue(TEXT("Imported actor found in test world."), Found);
		});
	});
}

/// @todo Cannot (yet) use Spec for tickign tests because I don't know how to make a Latent It
/// that waits until the game world reaches some pre-defiend time. With Latent Commands that is
/// handled automatically by checking the time on each call to Update returning false until the time
/// is right. What I need to know is, how do I abandon an LatentIt until the next tick?
#if 0
BEGIN_DEFINE_SPEC(
	FArchiveImporterToSingleAcgor_SingleSphereSpec,
	"AGXUnreal.ArchiveImporterToSingleActor.SingleSphere.Spec",
	AgxAutomationCommon::DefaultTestFlags)
const TCHAR* MapName = TEXT("Test_ArchiveImport");
const TCHAR* ArchiveName = TEXT("single_sphere.agx");
UWorld* World = nullptr;
AActor* Contents = nullptr;
UAGX_RigidBodyComponent* Sphere = nullptr;
FVector SphereStartPosition;
END_DEFINE_SPEC(FArchiveImporterToSingleAcgor_SingleSphereSpec)

void FArchiveImporterToSingleAcgor_SingleSphereSpec::Define()
{
	Describe(TEXT("Import single sphere"), [this]() {
		// Check world.
		BeforeEach([this]() {
			World = AgxAutomationCommon::GetTestWorld();
			TestNotNull(TEXT("The test world most not be null."), World);
			UE_LOG(LogAGX, Warning, TEXT("Got world %p."), (void*) World);
		});

		// Load map.
		BeforeEach([this]() {
			UE_LOG(LogAGX, Warning, TEXT("Opening map '%s'"), MapName);
			GEngine->Exec(World, *FString::Printf(TEXT("open %s"), MapName));
		});

		// Import archive.
		BeforeEach([this]() {
			FString ArchiveFilePath = AgxAutomationCommon::GetArchivePath(ArchiveName);
			Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
			TestNotNull(TEXT("Contents"), Contents);
			TArray<UActorComponent*> Components;
			Contents->GetComponents(Components, false);
			TestEqual(TEXT("Number of imported components"), Components.Num(), 3);
			Sphere =
				AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
			TestNotNull(TEXT("Sphere body"), Sphere);
			SphereStartPosition = Sphere->GetComponentLocation();
			UE_LOG(LogAGX, Warning, TEXT("Imported archive '%s'."), *ArchiveFilePath);
		});

		// Check contents.
		It(TEXT("should have imported a sphere"),
		   [this]() { TestNotNull(TEXT("Imported sphere"), Sphere); });

		LatentIt(TEXT("should have a falling sphere"), [this](const FDoneDelegate& Done) {

		});
	});
}
#endif

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
 * Latent Command that waits until the given World reaches the given time, in seconds.
 * @todo Replace with AgxAutomationCommon::FWaitUntilTime once we're confident that this won't cause
 * and infinite wait.
 */
class FWaitUntilTime final : public IAutomationLatentCommand
{
public:
	FWaitUntilTime(UWorld*& InWorld, float InTime, FAutomationTestBase& InTest)
		: World(InWorld)
		, Time(InTime)
		, Test(InTest)
	{
	}

	virtual ~FWaitUntilTime()
	{
	}

	virtual bool Update()
	{
		++NumUpdates;
		if (NumUpdates > 1000)
		{
			Test.AddError("Did not reach the time event after many ticks. Giving up.");
			return true;
		}
		return World->GetTimeSeconds() >= Time;
	}

private:
	UWorld*& World;
	float Time;
	int32 NumUpdates = 0;
	FAutomationTestBase& Test;
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
	float EndTime = -1.0f;
	int32 NumTicks = 0;

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace AgxAutomationCommon;
		BAIL_TEST_IF_CANT_SIMULATE()
		World = AgxAutomationCommon::GetTestWorld();
		Simulation = UAGX_Simulation::GetFrom(World);
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")))
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand())
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("single_sphere.agx", Contents, *this))
		ADD_LATENT_AUTOMATION_COMMAND(FCheckSingleSphereImportedCommand(*this))

		/// @todo Using local game time waiter until we know it works as intended.
		// ADD_LATENT_AUTOMATION_COMMAND(FTickUntilCommand(World, 1.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilTime(World, 1.0f, *this));

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
	Test.StartAgxTime = Test.Simulation->GetTimeStamp();
	Test.StartUnrealTime = Test.World->GetTimeSeconds();
	Test.TestEqual("World and AGX times should be equal", Test.StartAgxTime, Test.StartUnrealTime);

	return true;
}

bool FCheckSphereHasMoved::Update()
{
	if (Test.SphereBody == nullptr)
	{
		return true;
	}
	FVector EndPosition = Test.SphereBody->GetComponentLocation();
	Test.TestEqual("Sphere should move", EndPosition, Test.StartPosition);

	FVector EndVelocity = Test.SphereBody->Velocity;
	Test.TestEqual("Sphere should accelerate due to gravity", EndVelocity, Test.StartVelocity);
	return true;
}

#if 0
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FBodyMoved, FBodyMovedParams, Params);

bool FBodyMoved::Update()
{
	Params.Update();
	if (Params.UpdateCounter > 1000)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Did not reach target time even after %d updates. Bailing"),
			Params.UpdateCounter);
		return true;
	}

	UE_LOG(LogAGX, Warning, TEXT("Update at time %f."), Params.World->GetTimeSeconds());

	if (Params.World->GetTimeSeconds() < Params.EndTime)
	{
		FVector NewPosition =
			Params.Body->GetOwner()->GetActorLocation(); // GetComponentLocation();
		UE_LOG(
			LogAGX, Warning, TEXT("Still stepping when body is at (%f, %f, %f)."), NewPosition.X,
			NewPosition.Y, NewPosition.Z);

		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(Params.Body->GetOwner());
		if (Simulation != nullptr)
		{
			UE_LOG(LogAGX, Warning, TEXT("Simulation time is %d."), Simulation->GetTimeStamp());
		}
		else
		{
			UE_LOG(LogAGX, Warning, TEXT("DO NOT HAVE A SIMULATION!"));
		}

		return false;
	}

	FVector NewPosition = Params.Body->GetComponentLocation();
	FVector Displacement = NewPosition - Params.StartPosition;
	UE_LOG(
		LogAGX, Warning, TEXT("Body has moved (%f, %f, %f)."), Displacement.X, Displacement.Y,
		Displacement.Z);
	return true;
}

// FAutomationTestBase
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FLoadSingleSphereArchive, FAutomationTestBase*, GameTest);

bool FLoadSingleSphereArchive::Update()
{
	UWorld* CurrentWorld = FAGX_EditorUtilities::GetCurrentWorld();
	GameTest->TestNotNull(TEXT("Current world"), CurrentWorld);

	FString ArchiveFilePath = AgxAutomationCommon::GetArchivePath(TEXT("single_sphere.agx"));
	AActor* Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
	GameTest->TestNotNull(TEXT("Actor restored from archive"), Contents);
	if (Contents == nullptr)
	{
		return true;
	}

	TArray<UActorComponent*> Components;
	Contents->GetComponents(Components, false);
	GameTest->TestEqual(TEXT("Number of imported components"), Components.Num(), 3);
	USceneComponent* SceneRoot =
		AgxAutomationCommon::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));

	UAGX_RigidBodyComponent* BulletBody =
		AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
	UAGX_SphereShapeComponent* BulletShape =
		AgxAutomationCommon::GetByName<UAGX_SphereShapeComponent>(Components, TEXT("bullet_1"));

	GameTest->TestNotNull(TEXT("DefaultSceneRoot"), SceneRoot);
	GameTest->TestNotNull(TEXT("Bullet"), BulletBody);
	GameTest->TestNotNull(TEXT("Bullet_1"), BulletShape);

	float Mass = BulletBody->Mass;
	FVector LinearVelocity = BulletBody->Velocity;
	FVector AngularVelocity = BulletBody->AngularVelocity;
	EAGX_MotionControl MotionControl = BulletBody->MotionControl;
	uint8_t bTransformRootComponent = BulletBody->bTransformRootComponent;
	bool bHasNative = BulletBody->HasNative();
	UWorld* BodyWorld = BulletBody->GetWorld();

	UE_LOG(
		LogAGX, Warning, TEXT("Body has velocity (%f, %f, %f)."), LinearVelocity.X,
		LinearVelocity.Y, LinearVelocity.Z);
	GameTest->TestEqual(TEXT("Sphere mass"), Mass, 100.0f);
	GameTest->TestEqual(
		TEXT("Sphere linear velocity"), LinearVelocity, FVector(-4.73094f, 16.5768f, 10.9014f));

	/// \todo This fails. Partly because we can't just text-copy the 'Expected' part from agxViewer
	/// print-outs, and partly because of an actual bug in
	/// UAGX_RigidBodyComponent::GetAngularVelocity. It doesn't handle radians/degrees as it should.
	GameTest->TestEqual(
		TEXT("Sphere angular velocity"), AngularVelocity, FVector(17.7668f, 2.27498f, 7.87081f));
	GameTest->TestEqual(
		TEXT("Sphere motion control"), MotionControl, EAGX_MotionControl::MC_DYNAMICS);
	GameTest->TestFalse(TEXT("Sphere transform root component"), bTransformRootComponent);
	GameTest->TestFalse(TEXT("Sphere has native"), bHasNative); /// \todo Are we sure?
	GameTest->TestEqual(TEXT("Sphere world"), BodyWorld, CurrentWorld);

	UE_LOG(LogAGX, Warning, TEXT("Found %d components in SingleSphere."), Components.Num());
	for (auto& Component : Components)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("  Component named '%s' is of type '%s'."), *Component->GetName(),
			*Component->GetClass()->GetName())
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FDeallocateSingleSphereTestState, FSingleSphereTestState*, SingleSphereTestState);

bool FDeallocateSingleSphereTestState::Update()
{
	delete SingleSphereTestState;
	return true;
}

bool FArchiveImporterToSingleActor_SingleSphereTest::RunTest(const FString& Parameters)
{
	// Reference to the test level: World'/Game/Tests/Test_ArchiveImport.Test_ArchiveImport'
	FString MapName = TEXT("Test_ArchiveImport");

	/// \todo I don't know what I should do instead of new/delete here.
	FSingleSphereTestState* SingleSphereTestState = new FSingleSphereTestState();

	ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(MapName));
	ADD_LATENT_AUTOMATION_COMMAND(FLoadSingleSphereArchive(this));
	ADD_LATENT_AUTOMATION_COMMAND(FBodyMoved(FBodyMovedParams(SingleSphereTestState, 1.0f)));
	ADD_LATENT_AUTOMATION_COMMAND(FDeallocateSingleSphereTestState(SingleSphereTestState));

	UE_LOG(LogAGX, Warning, TEXT("End of RunTest.\n\n\n\n"))
	return true;
}
#endif
