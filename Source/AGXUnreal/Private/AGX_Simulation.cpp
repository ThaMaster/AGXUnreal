#include "AGX_Simulation.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Stepper.h"
#include "AGX_LogCategory.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

#include <algorithm>

FAGX_Statistics UAGX_Simulation::GetStatistics()
{
	check(HasNative());
	if (!bEnableStatistics)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_Simulation::GetStepForwardTime called while statistics gathering is "
				 "disabled. Enable in Project Settings > Plugins > AGX Dynamics > Statistics."));
		return FAGX_Statistics();
	}

	return NativeBarrier.GetStatistics();
}

void UAGX_Simulation::AddRigidBody(UAGX_RigidBodyComponent* Body)
{
	check(Body != nullptr);
	EnsureStepperCreated();
	NativeBarrier.AddRigidBody(Body->GetNative());
}

void UAGX_Simulation::AddShape(UAGX_ShapeComponent* Shape)
{
	/// \note It's not entirely clear that we want to allow the user to create
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
	EnsureStepperCreated();
	NativeBarrier.AddTerrain(Terrain->GetNative());
}

void UAGX_Simulation::SetDisableCollisionGroupPair(const FName& Group1, const FName& Group2)
{
	EnsureStepperCreated();
	NativeBarrier.SetDisableCollisionGroupPair(Group1, Group2);
}

void UAGX_Simulation::SetNumPpgsIterations(int32 NumIterations)
{
	NumPpgsIterations = NumIterations;
	if (HasNative())
	{
		NativeBarrier.SetNumPpgsIterations(NumIterations);
	}
}

int32 UAGX_Simulation::GetNumPpgsIterations()
{
	if (HasNative())
	{
		check(NumPpgsIterations == NativeBarrier.GetNumPpgsIterations());
	}
	return NumPpgsIterations;
}

void UAGX_Simulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NativeBarrier.AllocateNative();
	check(HasNative()); /// \todo Consider better error handling.

	/// \todo Set time step here.

	if (bOverridePPGSIterations)
	{
		if (NumPpgsIterations < 1)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Clamping the number of PPGS solver iterations from %d to 1. Set the number of "
					"iterations to a positive value in Project Settings > Plugins > AGX Dynamics > "
					"Solver."),
				NumPpgsIterations);
			NumPpgsIterations = 1;
		}
		NativeBarrier.SetNumPpgsIterations(NumPpgsIterations);

		// Note that AGX Dynamics' Terrain can change the number of PPGS iterations. AAGX_Terrain
		// is responsible for restoring it to the value we set here.
	}

	/// \todo Set gravity here.

	NativeBarrier.SetStatisticsEnabled(bEnableStatistics);

	if (bRemoteDebugging)
	{
		NativeBarrier.EnableRemoteDebugging(RemoteDebuggingPort);
	}
}

void UAGX_Simulation::Deinitialize()
{
	Super::Deinitialize();
	if (!HasNative())
	{
		return;
	}
	NativeBarrier.SetStatisticsEnabled(false);
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

namespace agx_simulation_helpers
{
	void WriteInitialStateArchive(const FString& ExportPath, UAGX_Simulation& Simulation)
	{
		if (ExportPath.IsEmpty())
		{
			// No path specified, default to ProjectUserDir.
			WriteInitialStateArchive(FPaths::ProjectUserDir(), Simulation);
		}
		else if (FPaths::DirectoryExists(ExportPath))
		{
			// Exporting to a directory. Name archive based on current level.
			FString LevelName = UGameplayStatics::GetCurrentLevelName(&Simulation);
			if (LevelName.IsEmpty())
			{
				LevelName = TEXT("InitialState");
			}
			FString FileName = LevelName + TEXT(".agx");
			WriteInitialStateArchive(FPaths::Combine(ExportPath, FileName), Simulation);
		}
		else
		{
			// Exporting to a file. Must be a valid AGX Dynamics archive name.
			if (!(ExportPath.EndsWith(".agx") || ExportPath.EndsWith(".aagx")))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Cannot export initial state to archive: Export path '%s' does not "
						 "specify a "
						 "'.agx' or '.aagx' path."),
					*ExportPath)
				return;
			}
			FString FullPath = FPaths::ConvertRelativePathToFull(ExportPath);
			Simulation.WriteAGXArchive(FullPath);
		}
	}
}

void UAGX_Simulation::Step(float DeltaTime)
{
	using namespace agx_simulation_helpers;
#if WITH_EDITORONLY_DATA
	if (bExportInitialState)
	{
		// Is there a suitable callback we can use instead of checking before every step?
		bExportInitialState = false;
		WriteInitialStateArchive(ExportPath, *this);
	}
#endif

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:UAGX_Simulation::Step"));
	switch (StepMode)
	{
		case SM_CATCH_UP_IMMEDIATELY:
			StepCatchUpImmediately(DeltaTime);
			break;
		case SM_CATCH_UP_OVER_TIME:
			StepCatchUpOverTime(DeltaTime);
			break;
		case SM_CATCH_UP_OVER_TIME_CAPPED:
			StepCatchUpOverTimeCapped(DeltaTime);
			break;
		case SM_DROP_IMMEDIATELY:
			StepDropImmediately(DeltaTime);
			break;
		default:
			UE_LOG(LogAGX, Error, TEXT("Unknown step mode: %d"), StepMode);
	}
}

void UAGX_Simulation::StepCatchUpImmediately(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	while (DeltaTime >= TimeStep)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
		NativeBarrier.Step();
		DeltaTime -= TimeStep;
	}
	LeftoverTime = DeltaTime;
}

void UAGX_Simulation::StepCatchUpOverTime(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	// Step up to two times.
	for (int i = 0; i < 2; i++)
	{
		if (DeltaTime >= TimeStep)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
			NativeBarrier.Step();
			DeltaTime -= TimeStep;
		}
	}

	LeftoverTime = DeltaTime;
}

void UAGX_Simulation::StepCatchUpOverTimeCapped(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	// Step up to two times.
	for (int i = 0; i < 2; i++)
	{
		if (DeltaTime >= TimeStep)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
			NativeBarrier.Step();
			DeltaTime -= TimeStep;
		}
	}

	// Cap the LeftoverTime according to the TimeLagCap.
	LeftoverTime = std::min(DeltaTime, TimeLagCap);
}

void UAGX_Simulation::StepDropImmediately(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	if (DeltaTime >= TimeStep)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
		NativeBarrier.Step();
		DeltaTime -= TimeStep;
	}

	// Keep LeftoverTime updated in case the information is needed in the future.
	LeftoverTime = DeltaTime;
}

float UAGX_Simulation::GetTimeStamp() const
{
	return NativeBarrier.GetTimeStamp();
}

void UAGX_Simulation::SetTimeStamp(float NewTimeStamp)
{
	NativeBarrier.SetTimeStamp(NewTimeStamp);
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

#if WITH_EDITOR
bool UAGX_Simulation::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
) const
{
	// Time Lag Cap should only be editable when step mode SM_CATCH_UP_OVER_TIME_CAPPED is used.
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UAGX_Simulation, TimeLagCap))
	{
		return StepMode == SM_CATCH_UP_OVER_TIME_CAPPED;
	}

	return Super::CanEditChange(InProperty);
}
#endif

void UAGX_Simulation::EnsureStepperCreated()
{
	/// \todo Calling GetWorld() from UAX_Simulation::Initialize returns the wrong
	/// world when running an executable using this plugin. The reason is not clear.
	/// Therefore, the GetWorld()->SpawnActor call is made here, after Initialize has been run.
	if (!StepperCreated)
	{
		GetWorld()->SpawnActor(AAGX_Stepper::StaticClass());
		/// \todo Instead of creating an Actor for Step triggering, one may use
		///       FTickableObjectBase or FTickFunction. It's not clear to me how to
		///       use these other classes.

		StepperCreated = true;
	}
}
