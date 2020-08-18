
// AGXUnreal includes.
#include "AGX_ArchiveImporterToSingleActor.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
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
	if (ArchiveName.IsEmpty())
	{
		Test.AddError(TEXT("FImportArchiveSingleActorCommand not given an archive to import."));
		return true;
	}
	FString ArchiveFilePath = AgxAutomationCommon::GetArchivePath(ArchiveName);
	if (ArchiveFilePath.IsEmpty())
	{
		Test.AddError(FString::Printf(TEXT("Did not find an archive name '%s'."), *ArchiveName));
		return true;
	}
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
			  TEXT("AGXUnreal.ArchiveImporterToSingleActor.EmptyScene"))
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
			  TEXT("AGXUnreal.ArchiveImporterToSingleActor.SingleSphere"))
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

		// See comment in FArchiveImporterToSingleActor_EmptySceneTest.
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

	// Diffuse color.
	{
		UMaterialInterface* Material = SphereShape->GetMaterial(0);
		if (Material == nullptr)
		{
			Test.AddError(TEXT("Imported sphere shape did not get a material"));
			/// @todo How should we write this? Just continuing with the test will give segmentation
			/// fault, returning will skip the rest of this test (might be ok), but adding an if
			/// (Material != nullptr) {} will produce too much indentation (possibly, try it).
			return true;
		}
		FMaterialParameterInfo Info;
		Info.Name = TEXT("Diffuse");
		FLinearColor Actual;
		if (Material->GetVectorParameterValue(Info, Actual, true))
		{
			// Color set in single_sphere.agxPy and imported to a Material Instance.
			FLinearColor Expected(1.0f, 0.3f, 0.3f, 1.0f);
			Test.TestEqual(TEXT(""), Actual, Expected);
		}
		else
		{
			// Color not set in a Material Instance. The default material should be used.
			if (GIsEditor)
			{
				// We expect to get here when running the test outside of the editor because then
				// we're not allowed to create new Material Instance assets. But if we get here when
				// in the editor then failing to create the material asset is a test failure.
				Test.AddError(TEXT(
					"The imported render material does not have an overridden diffuse color."));
			}
			if (Material->GetVectorParameterValue(Info, Actual, false))
			{
				// The default color that should be given to imported shapes that don't come with
				// a render material.
				FLinearColor Expected(0.896269f, 0.258183f, 0.000000f, 1.000000f);
				Test.TestEqual(
					*FString::Printf(
						TEXT("Actual: %s. Expected: %s."), *Actual.ToString(),
						*Expected.ToString()),
					Actual.ToString(), Expected.ToString());
				/// \@todo Don't compare strings, either find the actual expected color values or
				/// add an FLinearColor TestEqual overload that takes a Tolerance in
				/// AgxAutomationCommon.
			}
			else
			{
				Test.AddError(TEXT("Sphere shape's Material does not have a diffuse parameter."));
			}
		}
	}

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

//
// SimpleTrimesh test starts here.
//

class FArchiveImporterToSingleActor_SimpleTrimeshTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckSimpleTrimeshImportedCommand, FArchiveImporterToSingleActor_SimpleTrimeshTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearSimpleTrimeshImportedCommand, FArchiveImporterToSingleActor_SimpleTrimeshTest&, Test);

class FArchiveImporterToSingleActor_SimpleTrimeshTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToSingleActor_SimpleTrimeshTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToSingleActor_SimpleTrimeshTest"),
			  TEXT("AGXUnreal.ArchiveImporterToSingleActor.SimpleTrimesh"))
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

		// See comment in FArchiveImporterToSingleActor_EmptySceneTest.
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
	FArchiveImporterToSingleActor_SimpleTrimeshTest ArchiveImporterToSingleActor_SimpleTrimeshTest;
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

	// StaticMeshComponent.
	for (auto _ : {1}) // This weird for-loop is a hacky way to get break;. Why not a function?
	{
		const TArray<USceneComponent*>& Children = TrimeshShape->GetAttachChildren();
		Test.TestEqual(TEXT("TrimeshShape child components"), Children.Num(), 1);
		if (Children.Num() != 1)
		{
			break;
		}
		USceneComponent* Child = Children[0];
		Test.TestNotNull(TEXT("Child"), Child);
		if (Child == nullptr)
		{
			break;
		}
		UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child);
		Test.TestNotNull(TEXT("Trimesh asset"), Mesh);
		if (Mesh == nullptr)
		{
			break;
		}
		Test.TestEqual(
			TEXT("The StaticMesh should be a child of the TrimeshShape"), Mesh, StaticMesh);
	}

	UE_LOG(LogAGX, Display, TEXT("End of SimpleTrimesh import test."));

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

#if 1
	// The is the sledgehammer appraoch. I don't expect Unreal Engine to like this.
	// An attempt at deleting the assets that are created by the SimpleTrimesh test.
	// I have tried a few variantes (see below) to do this cleanly via the Engine API but I don't
	// know what I'm doing and it always crashes. Doing filesystem delete for now. Nothing is
	// referencing theses assets and nothing ever will again, and the engine will shut down shortly,
	// if the tests are being run from the command line.
	//
	// I'm just worried that the wrong directory may be deleted in some circumstances.

	const FString Root = FPaths::ProjectContentDir();
	const FString ImportsLocal = TEXT("ImportedAgxArchives/simple_trimesh_build");
	const FString ImportsFull = FPaths::Combine(Root, ImportsLocal);
	const FString ImportsAbsolute = FPaths::ConvertRelativePathToFull(ImportsFull);
	if (ImportsFull == Root)
	{
		Test.AddError(
			"Cannot clean SimpleTrimesh assets: Test directory is the same as the root content "
			"directory.");
		return true;
	}
	if (ImportsAbsolute.IsEmpty())
	{
		Test.AddError("Cannot clean SimpleTrimesh assets: Did not find content directory.");
		return true;
	}
	if (!FPaths::DirectoryExists(ImportsAbsolute))
	{
		Test.AddError("Cannot clean SimpleTrimeshAssets: Content directory does not exist.");
		return true;
	}

	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *ImportsAbsolute, TEXT("*"), true, true);
	if (Files.Num() != 2)
	{
		Test.AddError(
			TEXT("Cannot clean SimpleTrimeshAssets: Test directory contains an unexpected number "
				 "of files or directories."));
		return true;
	}
	TArray<FString> ExpectedFiles;
	ExpectedFiles.Add("StaticMeshs");
	ExpectedFiles.Add("simple_trimesh.uasset");
	for (const FString& File : Files)
	{
		const FString Name = FPaths::GetCleanFilename(File);
		if (!ExpectedFiles.Contains(Name))
		{
			Test.AddError(
				"Cannot clean SimpleTrimeshAssets: Test directory contains unexpected file or "
				"directory.");
			return true;
		}
	}
	IFileManager::Get().DeleteDirectory(*ImportsAbsolute, true, true);
#elif 0
	// An attempt at deleting the StaticMesh asset.
	// Crashes on GEditor->Something because GEditor is nullptr.
	/// @todo The path for this particular run may be different, may have a _# suffix. How do I find
	/// the path for this particular run?
	const TCHAR* MeshPath = TEXT(
		"StaticMesh'/Game/ImportedAgxArchives/simple_trimesh_build/StaticMeshs/"
		"simple_trimesh.simple_trimesh'");
	UObject* Asset = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, MeshPath);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete imported asset '%s': The asset was not found by StaticLoadObject."),
			MeshPath)
		return true;
	}
	TArray<FAssetData> Assets;
	Assets.Add(FAssetData(Asset));
	ObjectTools::DeleteAssets(Assets, false);
#else
	// An attempt at deleting the StaticMesh asset.
	// Crashes on the first run, and does nothing for subsequent runs.
	/// @todo The path for this particular run may be different, may have a _# suffix. How do I find
	/// the path for this particular run?
	const TCHAR* MeshPath = TEXT(
		"StaticMesh'/Game/ImportedAgxArchives/simple_trimesh_build/StaticMeshs/"
		"simple_trimesh.simple_trimesh'");
	UObject* Asset = StaticLoadObject(UStaticMesh::StaticClass(), nullptr, MeshPath);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete imported asset '%s': The asset was not found by StaticLoadObject."),
			MeshPath)
		return true;
	}
	Asset->MarkPendingKill();
#endif

	return true;
}

//
// RenderMaterial test starts here.
//

class FArchiveImporterToSingleActor_RenderMaterialTest;

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckRenderMaterialImportedCommand, FArchiveImporterToSingleActor_RenderMaterialTest&, Test);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FClearRenderMaterialImportedCommand, FArchiveImporterToSingleActor_RenderMaterialTest&, Test);

class FArchiveImporterToSingleActor_RenderMaterialTest final
	: public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FArchiveImporterToSingleActor_RenderMaterialTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FArchiveImporterToSingleActor_RenderMaterialTest"),
			  TEXT("AGXUnreal.Editor.ArchiveImporterToSingleActor.RenderMaterial"))
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
	FArchiveImporterToSingleActor_RenderMaterialTest
		ArchiveImporterToSingleActor_RenderMaterialTest;
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
		UMaterialInterface& Material, const TCHAR* ParameterName, const FLinearColor& Expected,
		FAutomationTestBase& Test)
	{
		FMaterialParameterInfo Info;
		Info.Name = ParameterName;
		FLinearColor Actual;
		if (!Material.GetVectorParameterValue(Info, Actual, false))
		{
			Test.AddError(FString::Printf(
				TEXT("Could not get parameter '%s' from material '%s'."), ParameterName,
				*Material.GetName()));
			return;
		}
		AgxAutomationCommon::TestEqual(
			Test, *FString::Printf(TEXT("%s in %s"), ParameterName, *Material.GetName()), Actual,
			Expected);
	}

	struct FMaterialParameters
	{
		FLinearColor Ambient {0.01f, 0.0028806f, 0.0f, 1.0f};
		FLinearColor Diffuse {0.8962694f, 0.258183f, 0.0f, 1.0f};
		FLinearColor Emissive {0.0f, 0.0f, 0.0f, 1.0f};
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

	// Get all the imported components.
	TArray<UActorComponent*> Components;
	Test.Contents->GetComponents(Components, false);
	Test.TestEqual(TEXT("Number of imported components"), Components.Num(), 10);

	auto GetSphere = [&Components](const TCHAR* Name) -> UAGX_SphereShapeComponent* {
		return GetByName<UAGX_SphereShapeComponent>(Components, Name);
	};

	// Get the components we know should be there.
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

	if (SceneRoot == nullptr || Body == nullptr || Ambient == nullptr || Diffuse == nullptr ||
		Emissive == nullptr || Shininess == nullptr || AmbientDiffuse == nullptr ||
		AmbientEmissive == nullptr || DiffuseShininessLow == nullptr ||
		DiffuseShininessHigh == nullptr)
	{
		Test.AddError(TEXT("At least one required object was nullptr, cannot continue."));
		return true;
	}

	// Ambient.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = {0.32f, 0.85f, 0.21f, 1.0f};
		TestMaterial(*Ambient, Parameters, Test);
	}
	// Diffuse.
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = {0.80f, 0.34f, 0.21f, 1.0f};
		TestMaterial(*Diffuse, Parameters, Test);
	}
	// Emissive.
	{
		FMaterialParameters Parameters;
		Parameters.Emissive = {0.98f, 0.94f, 0.76f, 1.0f};
		TestMaterial(*Emissive, Parameters, Test);
	}
	// AmbientDiffuse
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = {0.81f, 0.34f, 0.26f, 1.0f};
		Parameters.Diffuse = {0.32f, 0.28f, 0.67f, 1.0f};
		TestMaterial(*AmbientDiffuse, Parameters, Test);
	}
	// AmbientEmissive.
	{
		FMaterialParameters Parameters;
		Parameters.Ambient = {0.32f, 0.34f, 0.54f, 1.0f};
		Parameters.Emissive = {0.21f, 0.17f, 0.23f, 1.0f};
		TestMaterial(*AmbientEmissive, Parameters, Test);
	}
	// DiffuseShininessLow
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = {0.65f, 0.74f, 0.48f, 1.0f};
		Parameters.Shininess = 0.0f;
		TestMaterial(*DiffuseShininessLow, Parameters, Test);
	}
	// DiffuseShininessHigh
	{
		FMaterialParameters Parameters;
		Parameters.Diffuse = {0.65f, 0.74f, 0.48f, 1.0f};
		Parameters.Shininess = 1.0f;
		TestMaterial(*DiffuseShininessHigh, Parameters, Test);
	}

	return true;
}

bool FClearRenderMaterialImportedCommand::Update()
{
	return true;
}

// WITH_DEV_AUTOMATION_TESTS
#endif
