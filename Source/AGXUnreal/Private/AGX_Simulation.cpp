#include "AGX_Simulation.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_StaticMeshComponent.h"
#include "AGX_Stepper.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Materials/AGX_ContactMaterialInstance.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/ShapeBarrier.h"
#include "Terrain/AGX_Terrain.h"
#include "Tires/AGX_TireComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_EnvironmentUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Wire/AGX_WireComponent.h"

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

namespace AGX_Simulation_helpers
{
	template <typename T>
	void Add(UAGX_Simulation& Sim, T& ActorOrComponent)
	{
		if (!Sim.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Add was called in a UAGX_Simulation that does not have a native."));
			return;
		}

		if (!ActorOrComponent.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to add '%s' in '%s' that does not have a native to the "
					 "UAGX_Simulation."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		const bool Result = Sim.GetNative()->Add(*ActorOrComponent.GetNative());
		if (!Result)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to add '%s' in '%s' to the simulation. FSimulationBarrier::Add returned "
					 "false. The Log category AGXDynamicsLog may contain more information about "
					 "the failure."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
		}
	}

	template <typename T>
	void Remove(UAGX_Simulation& Sim, T& ActorOrComponent)
	{
		if (!Sim.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Remove was called in a UAGX_Simulation that does not have a native."));
			return;
		}

		if (!ActorOrComponent.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to remove '%s' in '%s' that does not have a native from the "
					 "UAGX_Simulation."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		const bool Result = Sim.GetNative()->Remove(*ActorOrComponent.GetNative());
		if (!Result)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to remove '%s' in '%s' to the simulation. FSimulationBarrier::Remove "
					 "returned false. The Log category AGXDynamicsLog may contain more information "
					 "about the failure."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
		}
	}
}

void UAGX_Simulation::Add(UAGX_ConstraintComponent& Constraint)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Constraint);
}

void UAGX_Simulation::Add(UAGX_ContactMaterialInstance& Material)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Remove was called in a UAGX_Simulation that does not have a native."));
		return;
	}

	if (!Material.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Contact Material '%s' to simulation but it does not have a native."),
			*Material.GetName());
		return;
	}

	if (!GetNative()->Add(*Material.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Contact Material '%s' to simulation but FSimulationBarrier::Add "
				 "returned false. The Log category AGXDynamicsLog may contain more information "
				 "about the failure."));
	}
}

void UAGX_Simulation::Add(UAGX_RigidBodyComponent& Body)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Body);
}

void UAGX_Simulation::Add(UAGX_ShapeComponent& Shape)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	UAGX_RigidBodyComponent* OwningBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(Shape);
	if (OwningBody != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape '%s' is owned by RigidBody '%s'. It should not be directly added to the "
				 "simulation."),
			*Shape.GetName(), *OwningBody->GetName());
		return;
	}
	AGX_Simulation_helpers::Add(*this, Shape);
}

void UAGX_Simulation::Add(UAGX_ShapeMaterialInstance& Shape)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Remove was called in a UAGX_Simulation that does not have a native."));
		return;
	}

	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Shape Material '%s' to simulation but it does not have a native."),
			*Shape.GetName());
		return;
	}

	if (!GetNative()->Add(*Shape.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Shape Material '%s' to simulation but FSimulationBarrier::Add "
				 "returned false. The Log category AGXDynamicsLog may contain more information "
				 "about the failure."));
	}
}

void UAGX_Simulation::Add(UAGX_StaticMeshComponent& Body)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Body);
}

void UAGX_Simulation::Add(AAGX_Terrain& Terrain)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Terrain);
}

void UAGX_Simulation::Add(UAGX_TireComponent& Tire)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Tire);
}

void UAGX_Simulation::Add(UAGX_WireComponent& Wire)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Wire);
}

void UAGX_Simulation::Remove(UAGX_ConstraintComponent& Constraint)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Constraint);
}

void UAGX_Simulation::Remove(UAGX_ContactMaterialInstance& Material)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Remove was called in a UAGX_Simulation that does not have a native."));
		return;
	}

	if (!Material.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Contact Material '%s' from simulation but it does not have a native."),
			*Material.GetName());
		return;
	}

	if (!GetNative()->Remove(*Material.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Contact Material '%s' from simulation but FSimulationBarrier::Remove"
				 "returned false. The Log category AGXDynamicsLog may contain more information about "
				 "the failure."));
	}
}

void UAGX_Simulation::Remove(UAGX_RigidBodyComponent& Body)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Body);
}

void UAGX_Simulation::Remove(UAGX_ShapeComponent& Shape)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();

	UAGX_RigidBodyComponent* OwningBody =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(Shape);
	if (OwningBody != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Shape '%s' is owned by RigidBody '%s'. It should not be directly removed from the "
				"simulation."),
			*Shape.GetName(), *OwningBody->GetName());
		return;
	}
	AGX_Simulation_helpers::Remove(*this, Shape);
}

void UAGX_Simulation::Remove(UAGX_ShapeMaterialInstance& Shape)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Remove was called in a UAGX_Simulation that does not have a native."));
		return;
	}

	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Shape Material '%s' from simulation but it does not have a "
				 "native."),
			*Shape.GetName());
		return;
	}

	if (!GetNative()->Remove(*Shape.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Shape Material '%s' from simulation but FSimulationBarrier::Remove "
				 "returned false. The Log category AGXDynamicsLog may contain more information about the "
				 "failure."));
	}
}

void UAGX_Simulation::Remove(UAGX_StaticMeshComponent& Body)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Body);
}

void UAGX_Simulation::Remove(AAGX_Terrain& Terrain)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Terrain);
}

void UAGX_Simulation::Remove(UAGX_TireComponent& Tire)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Tire);
}

void UAGX_Simulation::Remove(UAGX_WireComponent& Wire)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	AGX_Simulation_helpers::Remove(*this, Wire);
}

void UAGX_Simulation::SetEnableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool CanCollide)
{
	EnsureLicenseChecked();
	EnsureStepperCreated();
	NativeBarrier.SetEnableCollisionGroupPair(Group1, Group2, CanCollide);
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
	else
	{
		// The user has requested that we use the default number of PPGS solver iterations. Update
		// the setting to reflect the actual value set by AGX Dynamics. This will not change the
		// plugin settings since 'this' is now the in-game instance, not the CDO.
		NumPpgsIterations = NativeBarrier.GetNumPpgsIterations();
	}

	SetGravity();
	NativeBarrier.SetStatisticsEnabled(bEnableStatistics);
	NativeBarrier.SetTimeStep(TimeStep);

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

namespace AGX_Simulation_helpers
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
	using namespace AGX_Simulation_helpers;
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
		case SmCatchUpImmediately:
			StepCatchUpImmediately(DeltaTime);
			break;
		case SmCatchUpOverTime:
			StepCatchUpOverTime(DeltaTime);
			break;
		case SmCatchUpOverTimeCapped:
			StepCatchUpOverTimeCapped(DeltaTime);
			break;
		case SmDropImmediately:
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

TArray<FShapeContactBarrier> UAGX_Simulation::GetShapeContacts(const FShapeBarrier& Shape) const
{
	if (!HasNative())
	{
		return TArray<FShapeContactBarrier>();
	}

	return NativeBarrier.GetShapeContacts(Shape);
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
	// Time Lag Cap should only be editable when step mode SmCatchUpOverTimeCapped is used.
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UAGX_Simulation, TimeLagCap))
	{
		return StepMode == SmCatchUpOverTimeCapped;
	}

	return Super::CanEditChange(InProperty);
}
#endif

void UAGX_Simulation::EnsureStepperCreated()
{
	/// \todo Calling GetWorld() from UAX_Simulation::Initialize returns the wrong
	/// world when running an executable using this plugin. The reason is not clear.
	/// Therefore, the GetWorld()->SpawnActor call is made here, after Initialize has been run.
	if (!Stepper.IsValid())
	{
		Stepper = GetWorld()->SpawnActor<AAGX_Stepper>();
		/// \todo Instead of creating an Actor for Step triggering, one may use
		///       FTickableObjectBase or FTickFunction. It's not clear to me how to
		///       use these other classes.
	}
}

namespace
{
	void InvalidLicenseMessageBox()
	{
		FString Status;
		if (FAGX_EnvironmentUtilities::IsAgxDynamicsLicenseValid(&Status) == false)
		{
			FString Message =
				"Invalid AGX Dynamics license. Status: " + Status +
				"\n\nIt will not be possible to run simulations using the AGX "
				"Dynamics for Unreal plugin.\n\nTo get your license, visit us at www.algoryx.se";

			if (!FAGX_EnvironmentUtilities::IsSetupEnvRun())
			{
				const FString ResourcesPath =
					FAGX_EnvironmentUtilities::GetAgxDynamicsResourcesPath();
				Message += "\n\nThe AGX Dynamics license file should be placed in: " +
						   FPaths::Combine(ResourcesPath, FString("data"), FString("cfg"));
			}

#if WITH_EDITOR
			Message +=
				"\n\nNote that the Unreal Editor must be restarted after adding the license file.";
#endif

			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Message);
		}
	}
}

void UAGX_Simulation::EnsureLicenseChecked()
{
	// This function provides a mechanism for showing a message box to the user exactly one time in
	// case the AGX Dynamics license is invalid.
	if (!IsLicenseChecked)
	{
		InvalidLicenseMessageBox();
		IsLicenseChecked = true;
	}
}

void UAGX_Simulation::SetGravity()
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Error, TEXT("SetGravity failed, native object has not been allocated."));
		return;
	}

	switch (GravityModel)
	{
		case EAGX_GravityModel::Uniform:
			NativeBarrier.SetUniformGravity(UniformGravity);
			break;
		case EAGX_GravityModel::Point:
			NativeBarrier.SetPointGravity(PointGravityOrigin, PointGravityMagnitude);
			break;
		default:
			UE_LOG(LogAGX, Error, TEXT("SetGravity failed, unknown GravityModel."));
			break;
	}
}
