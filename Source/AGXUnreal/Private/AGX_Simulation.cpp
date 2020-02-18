#include "AGX_Simulation.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_ShapeComponent.h"
#include "AGX_Stepper.h"
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UAGX_Simulation::AddRigidBody(UAGX_RigidBodyComponent* Body)
{
	check(Body != nullptr);
	NativeBarrier.AddRigidBody(Body->GetNative());
}

void UAGX_Simulation::AddShape(UAGX_ShapeComponent* Shape)
{
	/// \todo It's not entirely clear that we want to allow the user to create
	/// Shapes that aren't part of a body. However, we have no obvious way to
	/// prevent it, so allowing it for now. Remove this member function if it
	/// causes problems.

	check(Shape != nullptr);
	UAGX_RigidBodyComponent* OwningBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*Shape);
	if (OwningBody != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape '%s' is owned by RigidBody '%s'. It should not be directly added to the "
				 "simulation."),
			*Shape->GetName(), *OwningBody->GetName());
		return;
	}
	NativeBarrier.AddShape(Shape->GetNative());
}

void UAGX_Simulation::AddTerrain(AAGX_Terrain* Terrain)
{
	check(Terrain != nullptr);
	NativeBarrier.AddTerrain(Terrain->GetNative());
}

void UAGX_Simulation::SetDisableCollisionGroupPair(const FName& Group1, const FName& Group2)
{
	UE_LOG(
		LogAGX, Verbose, TEXT("Disabling collision between groups: [%s - %s]"), *Group1.ToString(),
		*Group2.ToString());

	NativeBarrier.SetDisableCollisionGroupPair(Group1, Group2);
}

void UAGX_Simulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: new agxSDK::Simulation"));
	NativeBarrier.AllocateNative();
	check(HasNative()); /// \todo Consider better error handling.
	GetWorld()->SpawnActor(AAGX_Stepper::StaticClass());
	/// \todo Instead of creating an Actor for Step triggering, one may use
	///       FTickableObjectBase or FTickFunction. It's not clear to me how to
	///       use these other classes.

	if (bRemoteDebugging)
	{
		NativeBarrier.EnableRemoteDebugging(RemoteDebuggingPort);
	}
}

void UAGX_Simulation::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogAGX, Log, TEXT("AGX_CALL: delete agxSDK::Simulation"));
	NativeBarrier.ReleaseNative();
}

bool UAGX_Simulation::WriteAGXArchive(const FString& Filename) const
{
	if (!HasNative())
	{
		/// \todo Can we create a temporary Simulation, instantiate all the AGX
		/// Dynamics objects there, store, and then throw everything away?
		UE_LOG(LogAGX, Error, TEXT("No simulation available, cannot store AGX Dynamics archive."));
		return false;
	}

	return NativeBarrier.WriteAGXArchive(Filename);
}

bool UAGX_Simulation::HasNative() const
{
	return NativeBarrier.HasNative();
}

FSimulationBarrier* UAGX_Simulation::GetNative()
{
	check(NativeBarrier.HasNative()); // Invalid to call this function before starting game!

	return &NativeBarrier;
}

const FSimulationBarrier* UAGX_Simulation::GetNative() const
{
	check(NativeBarrier.HasNative()); // Invalid to call this function before starting game!

	return &NativeBarrier;
}

void UAGX_Simulation::Step(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	while (DeltaTime >= TimeStep)
	{
#if 0
		UE_LOG(LogAGX, Log, TEXT("AGX_CALL: agxSDK::Simulation::stepForward"));
#endif
		NativeBarrier.Step();
		DeltaTime -= TimeStep;
	}
	LeftoverTime = DeltaTime;
}

UAGX_Simulation* UAGX_Simulation::GetFrom(const UActorComponent* Component)
{
	if (!Component)
	{
		return nullptr;
	}

	return GetFrom(Component->GetOwner());
}

UAGX_Simulation* UAGX_Simulation::GetFrom(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = Actor->GetGameInstance();
	check(GameInstance);

	return GetFrom(GameInstance);
}

UAGX_Simulation* UAGX_Simulation::GetFrom(const UWorld* World)
{
	if (!World)
		return nullptr;

	if (World->IsGameWorld())
	{
		return GetFrom(World->GetGameInstance());
	}
	else
	{
		return GetMutableDefault<UAGX_Simulation>();
	}
}

UAGX_Simulation* UAGX_Simulation::GetFrom(const UGameInstance* GameInstance)
{
	if (!GameInstance)
		return nullptr;

	return GameInstance->GetSubsystem<UAGX_Simulation>();
}
