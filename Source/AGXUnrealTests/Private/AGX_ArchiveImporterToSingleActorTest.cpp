
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
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

/// \TODO GetTestWorld doesn't work, I have only ever seen it return nullptr.
/// What does it do, actually? When is it supposed to be used? Why doesn it work in
/// AutomationCommon.cpp?

// Copy of the hidden method GetAnyGameWorld() in AutomationCommon.cpp.
// Marked as temporary there, hence, this one is temporary, too.
UWorld* GetTestWorld()
{
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	if (WorldContexts.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("GEngine->GetWorldContexts() is empty."));
	}
	for (const FWorldContext& Context : WorldContexts)
	{
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) &&
			(Context.World() != nullptr))
		{
			return Context.World();
		}
	}

	return nullptr;
}

FString GetArchivePath(const TCHAR* ArchiveName)
{
	return FPaths::Combine(
		TEXT("/home/ibbles/workspace/Algoryx/AGX_Dynamics_archives"), ArchiveName);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArchiveImporterToSingleActor_EmptySceneTest,
	"AGXUnreal.ArchiveImporterToSingleActor.EmptyScene",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FArchiveImporterToSingleActor_EmptySceneTest::RunTest(const FString& Parameters)
{
	/// \todo Add the archive to some folder within the plugin and use some API call to find that
	/// path.
	FString ArchiveFilePath = GetArchivePath(TEXT("empty_scene.agx"));

	UWorld* TestWorld = GetTestWorld();
	TestNotNull(TEXT("UWorld fetched with GetTestWorld"), TestWorld);

	UWorld* CurrentWorld = FAGX_EditorUtilities::GetCurrentWorld();
	TestNotNull(TEXT("UWorld fetched with FAGX_EditorUtilities::GetCurrentWorld()"), CurrentWorld);

	TestEqual(
		TEXT("Worlds fetched from GetTestWorld and GetCurrentWorld"), CurrentWorld, TestWorld);

	AActor* Contents = AGX_ArchiveImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
	TestNotNull(TEXT("Contents restored with ImportAGXArchive"), Contents);
	if (Contents == nullptr)
	{
		return false;
	}

	TArray<AActor*> Children;
	Contents->GetAllChildActors(Children, true);
	TestEqual(TEXT("Number of child actors"), Children.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FArchiveImporterToSingleActor_SingleSphereTest,
	"AGXUnreal.ArchiveImporterToSingleActor.SingleSphere",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

template <typename T>
T* GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
{
	UActorComponent** Match = Components.FindByPredicate([Name](UActorComponent* Component) {
		return Cast<T>(Component) && Component->GetName() == Name;
	});

	return Match != nullptr ? Cast<T>(*Match) : nullptr;
}

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

	USceneComponent* SceneRoot = GetByName<USceneComponent>(Components, TEXT("DefaultSceneRoot"));
	UAGX_RigidBodyComponent* BulletBody =
		GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
	UAGX_SphereShapeComponent* BulletShape =
		GetByName<UAGX_SphereShapeComponent>(Components, TEXT("bullet_1"));

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
