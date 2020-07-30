
// AGXUnreal includes.
#include "AGX_ArchiveImporterToSingleActor.h"
#include "AGX_EditorUtilities.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Shapes/AGX_SphereShapeComponent.h"

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
 * A collection of helper functions used by our Automation tests.
 *
 * \todo Move these somewhere where all tests have access to them.
 */
namespace TestHelpers
{
	// Copy of the hidden method GetAnyGameWorld() in AutomationCommon.cpp.
	// Marked as temporary there, hence, this one is temporary, too.
	//
	/// \TODO GetTestWorld doesn't work, I have only ever seen it return nullptr.
	/// What does it do, actually? When is it supposed to be used? Why does it work in
	/// AutomationCommon.cpp?
	///
	/// Answer:
	/// It does work, but the world must be enabled/running/ticking first. The way to enable the
	/// world when running unit tests from the command line through Unreal Editor is to pass `-Game`
	/// on the command line to UE4Editor. When I do that I get the same UWorld pointer from both
	/// GetTestWorld and FAGX_EditorUtilities::GetCurrentWorld.
	UWorld* GetTestWorld()
	{
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		if (WorldContexts.Num() == 0)
		{
			UE_LOG(LogAGX, Warning, TEXT("GEngine->GetWorldContexts() is empty."));
			return nullptr;
		}
		for (const FWorldContext& Context : WorldContexts)
		{
			bool bIsPieOrGame =
				Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game;
			if (bIsPieOrGame && Context.World() != nullptr)
			{
				return Context.World();
			}
		}
		UE_LOG(
			LogAGX, Warning, TEXT("Non of the %d WorldContexts contain a PIE or Game world."),
			WorldContexts.Num());
		return nullptr;
	}

	/**
	 * Get the file system path to and AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const TCHAR* ArchiveName)
	{
		/// \todo Find where, if at all, Automation test AGX Dynamics archives should be stored.
		/// In the repository or downloaded as with test files for AGX Dynamics.
		/// \todo Find the proper path somehow. Likely using FPaths.
		return FPaths::Combine(
			TEXT("/home/ibbles/workspace/Algoryx/AGX_Dynamics_archives"), ArchiveName);
	}

	/**
	 * Get the file system path to and AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const FString& ArchiveName)
	{
		return GetArchivePath(*ArchiveName);
	}

	template <typename T>
	T* GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
	{
		UActorComponent** Match = Components.FindByPredicate([Name](UActorComponent* Component) {
			return Cast<T>(Component) && Component->GetName() == Name;
		});

		return Match != nullptr ? Cast<T>(*Match) : nullptr;
	}

	template <typename T>
	void GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name, T*& Out)
	{
		Out = GetByName<T>(Components, Name);
	}
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FLogWarningAgxCommand, FString, Message);
bool FLogWarningAgxCommand::Update()
{
	UE_LOG(LogAGX, Warning, TEXT("%s"), *Message);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FLogErrorAgxCommand, FString, Message);
bool FLogErrorAgxCommand::Update()
{
	UE_LOG(LogAGX, Error, TEXT("%s"), *Message);
	return true;
}

/**
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
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("Before the error")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogErrorAgxCommand(TEXT("At the error.")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("After the error.")));
	return true;
}

/**
 * Latent Command that tests that TestHelper::GetTestWorld and FAGX_EditorUtilities::GetCurrentWorld
 * return the same world.
 *
 * \note This could be implemented directly in the Test itself, instead of as a Latent Command. Done
 * this way for experimentation/learning purposes. Move the actual test code to the Test's RunTest
 * once we're confident in our ability to write both Tests and Latent Commands.
 */
class FCheckWorldsCommand final : public IAutomationLatentCommand
{
public:
	FCheckWorldsCommand(FAutomationTestBase& InTest)
		: Test(InTest)
	{
	}

	virtual bool Update() override
	{
		UWorld* TestWorld = TestHelpers::GetTestWorld();
		UWorld* CurrentWorld = FAGX_EditorUtilities::GetCurrentWorld();
		UE_LOG(LogAGX, Warning, TEXT("TestWorld:    %p"), (void*) TestWorld);
		UE_LOG(LogAGX, Warning, TEXT("CurrentWorld: %p"), (void*) CurrentWorld);
		Test.TestEqual(TEXT("Worlds"), TestWorld, CurrentWorld);
		Test.TestNotNull("TestWorld", TestWorld);
		Test.TestNotNull("CurrentWorld", CurrentWorld);
		return true;
	}

private:
	FAutomationTestBase& Test;
};

/**
 * Test that TestHelper::GetTestWorld and FAGX_EditorUtilities::GetCurrentWorld return the same
 * world.
 */
class FCheckWorldsTest final : public FAutomationTestBase
{
public:
	FCheckWorldsTest()
		: FAutomationTestBase(TEXT("FCheckWorldsTest"), false)
	{
	}

	uint32 GetTestFlags() const override
	{
		return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	}

	uint32 GetRequiredDeviceNum() const override
	{
		return 1;
	}

	FString GetBeautifiedTestName() const override
	{
		return TEXT("AGXUnreal.CheckWorlds");
	}

protected:
	void GetTests(
		TArray<FString>& OutBeutifiedNames, TArray<FString>& OutTestCommands) const override
	{
		UE_LOG(
			LogAGX, Warning, TEXT("This should not be called since this is not a complex test."));
		OutBeutifiedNames.Add(GetBeautifiedTestName());
		OutTestCommands.Add(FString());
	}

	bool RunTest(const FString& InParameter) override
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Running test '%s' with parameter '%s'."), *GetTestName(),
			*InParameter);

		ADD_LATENT_AUTOMATION_COMMAND(FCheckWorldsCommand(*this));
		return true;
	}
};

// We must create an instantiate of the test class for the testing framework to find it.
namespace
{
	FCheckWorldsTest CheckWorldsTest;
}

/**
 * Get the file system path to and AGX Dynamcis archive intended for Automation testing.
 * @param ArchiveName The name of the AGX Dynamics archive to find.
 * @return File system path to the AGX Dynamics archive.
 * @deprecated Use TestHelpers::GetArchivePath instead.
 */
UE_DEPRECATED(4.16, "Use TestHelpers::GetArchivePath")
FString GetArchivePath(const TCHAR* ArchiveName)
{
	/// \todo Find where, if at all, Automation test AGX Dynamics archives should be stored.
	/// In the repository or downloaded as with test files for AGX Dynamics.
	/// \todo Find the proper path somehow. Likely using FPaths.
	return FPaths::Combine(
		TEXT("/home/ibbles/workspace/Algoryx/AGX_Dynamics_archives"), ArchiveName);
}

/**
 * Latent Command that imports an AGX Dynamics archive.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FImportArchiveSingleActorCommand, FString, ArchiveName, AActor*&, Contents,
	FAutomationTestBase&, Test);
bool FImportArchiveSingleActorCommand::Update()
{
	Test.TestEqual(
		TEXT("TestWorld and CurrentWorld"), TestHelpers::GetTestWorld(),
		FAGX_EditorUtilities::GetCurrentWorld());

	FString ArchiveFilePath = TestHelpers::GetArchivePath(ArchiveName);
	Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
	Test.TestNotNull(TEXT("Contents"), Contents);

	return true;
}

/**
 * Latent Command testing that the empty scene was imported correctly.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FCheckEmptySceneImportCommand, AActor*&, Contents, FAutomationTestBase&, Test);
bool FCheckEmptySceneImportCommand::Update()
{
	UWorld* World = TestHelpers::GetTestWorld();
	Test.TestEqual(TEXT("The actor's world and the test world."), Contents->GetWorld(), World);

	TArray<UActorComponent*> Components;
	Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 1);
	USceneComponent* SceneRoot =
		TestHelpers::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
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
class FArchiveImporterToSingleActor_EmptySceneTest final : public FAutomationTestBase
{
public:
	FArchiveImporterToSingleActor_EmptySceneTest()
		: FAutomationTestBase("FArchiveImporterToSingleActor_EmptySceneTest", false)
	{
	}

	uint32 GetTestFlags() const override
	{
		return EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter;
	}

	uint32 GetRequiredDeviceNum() const override
	{
		return 1;
	}

	void GetTests(
		TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
	{
		OutBeautifiedNames.Add(GetBeautifiedTestName());
		OutTestCommands.Add(FString());
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(TEXT("Test_ArchiveImport")));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
		ADD_LATENT_AUTOMATION_COMMAND(
			FImportArchiveSingleActorCommand("empty_scene.agx", Contents, *this));
		ADD_LATENT_AUTOMATION_COMMAND(FCheckEmptySceneImportCommand(Contents, *this));
		return true;
	}

	FString GetBeautifiedTestName() const override
	{
		return TEXT("AGXUnreal.ArchiveImporterToSingleActor.EmptyScene");
	}

private:
	AActor* Contents;
};

namespace
{
	FArchiveImporterToSingleActor_EmptySceneTest ArchiveImporterToSingleActor_EmptySceneTest;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArchiveImporterToSingleActor_SingleSphereTest,
	"AGXUnreal.ArchiveImporterToSingleActor.SingleSphere",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

struct FSingleSphereTestState
{
	UAGX_RigidBodyComponent* Body;
};

struct FBodyMovedParams
{
	UAGX_RigidBodyComponent* Body;
	UWorld* World = nullptr;
	FVector StartPosition = {0.0f, 0.0f, 0.0f};
	float StartTime = 0.0f;
	float EndTime = 0.0f;

	FBodyMovedParams(FSingleSphereTestState* InTestState, float InDuration)
		: Body(InTestState->Body)
		, Duration(InDuration)
	{
	}

	void Update()
	{
		if (UpdateCounter == 0)
		{
			Init();
		}
		++UpdateCounter;
	}

	void Init()
	{
		World = Body->GetWorld();
		StartPosition = Body->GetComponentLocation();
		StartTime = World->GetTimeSeconds();
		EndTime = StartTime + Duration;

		UE_LOG(
			LogAGX, Warning, TEXT("Will wait until world time is %f. Right now it is %f."), EndTime,
			StartTime);
	}
	int UpdateCounter = 0;
	float Duration = 0.0f;
};

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

	FString ArchiveFilePath = GetArchivePath(TEXT("single_sphere.agx"));
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
		TestHelpers::GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));

	UAGX_RigidBodyComponent* BulletBody =
		TestHelpers::GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
	UAGX_SphereShapeComponent* BulletShape =
		TestHelpers::GetByName<UAGX_SphereShapeComponent>(Components, TEXT("bullet_1"));

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
