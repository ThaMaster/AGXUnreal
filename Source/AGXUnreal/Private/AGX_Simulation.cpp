// Copyright 2022, Algoryx Simulation AB.

#include "AGX_Simulation.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_StaticMeshComponent.h"
#include "AGX_Stepper.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Materials/AGX_ContactMaterialInstance.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/ShapeBarrier.h"
#include "Terrain/AGX_Terrain.h"
#include "Tires/AGX_TireComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "AGX_Environment.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_Stats.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

#include <algorithm>

void UAGX_Simulation::SetNumThreads(int32 InNumThreads)
{
	if (NumThreads < 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("The number of threads cannot be negative."));
		return;
	}

	NumThreads = InNumThreads;
	if (HasNative())
	{
		NativeBarrier.SetNumThreads(static_cast<uint32>(NumThreads));
	}
}

int32 UAGX_Simulation::GetNumThreads() const
{
	if (HasNative())
	{
		return NativeBarrier.GetNumThreads();
	}

	return NumThreads;
}

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
				TEXT("Tried to add '%s' in '%s' to Simulation that does not have a native."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		if (!ActorOrComponent.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to add '%s' in '%s' that does not have a native to Simulation."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		const bool Result = Sim.GetNative()->Add(*ActorOrComponent.GetNative());
		if (!Result)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to add '%s' in '%s' to Simulation. FSimulationBarrier::Add returned "
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
				TEXT("Tried to remove '%s' in '%s' from Simulation that does not have a native."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		if (!ActorOrComponent.HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to remove '%s' in '%s' that does not have a native from Simulation "),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
			return;
		}

		const bool Result = Sim.GetNative()->Remove(*ActorOrComponent.GetNative());
		if (!Result)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to remove '%s' in '%s' from Simulation. FSimulationBarrier::Remove "
					 "returned false. The Log category AGXDynamicsLog may contain more information "
					 "about the failure."),
				*ActorOrComponent.GetName(), *GetLabelSafe(ActorOrComponent.GetOwner()));
		}
	}
}

void UAGX_Simulation::Add(UAGX_ConstraintComponent& Constraint)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Constraint);
}

void UAGX_Simulation::Add(UAGX_RigidBodyComponent& Body)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Body);
}

void UAGX_Simulation::Add(UAGX_ShapeComponent& Shape)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Shape);
}

void UAGX_Simulation::Add(UAGX_ShapeMaterial& Shape)
{
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Shape Material '%s' to Simulation that does not have a native."),
			*Shape.GetName());
		return;
	}

	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Shape Material '%s' to Simulation but the Shape Material does not "
				 "have a native."),
			*Shape.GetName());
		return;
	}

	if (!GetNative()->Add(*Shape.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to add Shape Material '%s' to Simulation but FSimulationBarrier::Add "
				 "returned false. The Log category AGXDynamicsLog may contain more information "
				 "about the failure."),
			*Shape.GetName());
	}
}

void UAGX_Simulation::Add(UAGX_StaticMeshComponent& Body)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Body);
}

void UAGX_Simulation::Add(AAGX_Terrain& Terrain)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Terrain);
}

void UAGX_Simulation::Add(UAGX_TireComponent& Tire)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Tire);
}

void UAGX_Simulation::Add(UAGX_WireComponent& Wire)
{
	EnsureStepperCreated();
	AGX_Simulation_helpers::Add(*this, Wire);
}

void UAGX_Simulation::Remove(UAGX_ConstraintComponent& Constraint)
{
	AGX_Simulation_helpers::Remove(*this, Constraint);
}

void UAGX_Simulation::Remove(UAGX_RigidBodyComponent& Body)
{
	AGX_Simulation_helpers::Remove(*this, Body);
}

void UAGX_Simulation::Remove(UAGX_ShapeComponent& Shape)
{
	AGX_Simulation_helpers::Remove(*this, Shape);
}

void UAGX_Simulation::Remove(UAGX_ShapeMaterial& Shape)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Shape Material '%s' from a Simulation that does not have a "
				 "native."),
			*Shape.GetName());
		return;
	}

	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Shape Material '%s' from Simulation but the Shape Material does "
				 "not have a native."),
			*Shape.GetName());
		return;
	}

	if (!GetNative()->Remove(*Shape.GetNative()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to remove Shape Material '%s' from Simulation but "
				 "FSimulationBarrier::Remove returned false. The Log category AGXDynamicsLog may "
				 "contain more information about the failure."),
			*Shape.GetName());
	}
}

void UAGX_Simulation::Remove(UAGX_StaticMeshComponent& Body)
{
	AGX_Simulation_helpers::Remove(*this, Body);
}

void UAGX_Simulation::Remove(AAGX_Terrain& Terrain)
{
	AGX_Simulation_helpers::Remove(*this, Terrain);
}

void UAGX_Simulation::Remove(UAGX_TireComponent& Tire)
{
	AGX_Simulation_helpers::Remove(*this, Tire);
}

void UAGX_Simulation::Remove(UAGX_WireComponent& Wire)
{
	AGX_Simulation_helpers::Remove(*this, Wire);
}

void UAGX_Simulation::Register(UAGX_ContactMaterialInstance& Material)
{
	EnsureStepperCreated();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to register Contact Material '%s' to Simulation that does not have a "
				 "native."),
			*Material.GetName());
		return;
	}

	if (!Material.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to register Contact Material '%s' to Simulation but the Contact Material "
				 "does not have a native."),
			*Material.GetName());
		return;
	}

	const int32 Count = ++ContactMaterials.FindOrAdd(&Material);
	check(Count > 0);

	// When the count goes from 0 to 1, we add the Contact Material to the Simulation.
	if (Count == 1)
	{
		if (!GetNative()->Add(*Material.GetNative()))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to add Contact Material '%s' to Simulation but FSimulationBarrier::Add "
					 "returned false. The Log category AGXDynamicsLog may contain more information "
					 "about the failure."),
				*Material.GetName());
		}
	}
}

void UAGX_Simulation::Unregister(UAGX_ContactMaterialInstance& Material)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to unregister Contact Material '%s' from Simulation that does not have a "
				 "native."),
			*Material.GetName());
		return;
	}

	if (!Material.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to unregister Contact Material '%s' from Simulation but the Contact "
				 "Material does not have a native."),
			*Material.GetName());
		return;
	}

	if (!ContactMaterials.Contains(&Material) || ContactMaterials[&Material] <= 0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to unregister Contact Material '%s' from Simulation but the Contact "
				 "Material has not been registered."),
			*Material.GetName());
		return;
	}

	const int32 Count = --ContactMaterials[&Material];
	check(Count >= 0);

	// When the count goes down to 0, we remove the Contact Material from the Simulation.
	if (Count == 0)
	{
		if (!GetNative()->Remove(*Material.GetNative()))
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to remove Contact Material '%s' from Simulation but "
					 "FSimulationBarrier::Remove "
					 "returned false. The Log category AGXDynamicsLog may contain more information "
					 "about the failure."),
				*Material.GetName());
		}
	}
}

void UAGX_Simulation::SetEnableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool CanCollide)
{
	EnsureStepperCreated();
	NativeBarrier.SetEnableCollisionGroupPair(Group1, Group2, CanCollide);
}

void UAGX_Simulation::SetEnableContactWarmstarting(bool bEnable)
{
	bContactWarmstarting = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableContactWarmstarting(bEnable);
	}
}

bool UAGX_Simulation::GetEnableContactWarmstarting() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnableContactWarmstarting();
	}
	else
	{
		return bContactWarmstarting;
	}
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
	EnsureValidLicense();

	NativeBarrier.AllocateNative();
	check(HasNative()); /// \todo Consider better error handling.

	NativeBarrier.SetTimeStep(TimeStep);

	NativeBarrier.SetEnableContactWarmstarting(bContactWarmstarting);

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

	NativeBarrier.SetNumThreads(NumThreads);
	SetGravity();
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

#if WITH_EDITOR

void UAGX_Simulation::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_Simulation::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_Simulation::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bContactWarmstarting),
		[](ThisClass* This) { This->SetEnableContactWarmstarting(This->bContactWarmstarting); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NumThreads),
		[](ThisClass* This) { This->SetNumThreads(This->NumThreads); });
}

#endif

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

	void ReportStepStatistics(const FAGX_Statistics& Statistics)
	{
		// Last step only timers.
		SET_FLOAT_STAT(STAT_AGXD_StepForward_STEP, Statistics.StepForwardTime);
		double Unaccounted = Statistics.StepForwardTime;
		SET_FLOAT_STAT(STAT_AGXD_SpaceUpdate_STEP, Statistics.SpaceTime);
		Unaccounted -= Statistics.SpaceTime;
		SET_FLOAT_STAT(STAT_AGXD_DynamicsUpdate_STEP, Statistics.DynamicsSystemTime);
		Unaccounted -= Statistics.DynamicsSystemTime;
		SET_FLOAT_STAT(STAT_AGXD_PreCollide_STEP, Statistics.PreCollideTime);
		Unaccounted -= Statistics.PreCollideTime;
		SET_FLOAT_STAT(STAT_AGXD_ContactEvents_STEP, Statistics.ContactEventsTime);
		Unaccounted -= Statistics.ContactEventsTime;
		SET_FLOAT_STAT(STAT_AGXD_PreStep_STEP, Statistics.PreStepTime);
		Unaccounted -= Statistics.PreStepTime;
		SET_FLOAT_STAT(STAT_AGXD_PostStep_STEP, Statistics.PostStepTime);
		Unaccounted -= Statistics.PostStepTime;
		SET_FLOAT_STAT(STAT_AGXD_LastStep_STEP, Statistics.LastStepTime);
		Unaccounted -= Statistics.LastStepTime;
		SET_FLOAT_STAT(STAT_AGXD_Unaccounted_STEP, Unaccounted);

		// Counters.
		SET_DWORD_STAT(STAT_AGXD_NumBodies, Statistics.NumBodies);
		SET_DWORD_STAT(STAT_AGXD_NumConstraints, Statistics.NumConstraints);
		SET_DWORD_STAT(STAT_AGXD_NumContactConstraints, Statistics.NumContacts);
		SET_DWORD_STAT(STAT_AGXD_NumParticles, Statistics.NumParticles);
	}

	void AccumulateFrameStatistics(const FAGX_Statistics& Statistics)
	{
		// Entire frame timers.
		INC_FLOAT_STAT_BY(STAT_AGXD_StepForward_FRAME, Statistics.StepForwardTime);
		double Unaccounted = Statistics.StepForwardTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_SpaceUpdate_FRAME, Statistics.SpaceTime);
		Unaccounted -= Statistics.SpaceTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_DynamicsUpdate_FRAME, Statistics.DynamicsSystemTime);
		Unaccounted -= Statistics.DynamicsSystemTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_PreCollide_FRAME, Statistics.PreCollideTime);
		Unaccounted -= Statistics.PreCollideTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_ContactEvents_FRAME, Statistics.ContactEventsTime);
		Unaccounted -= Statistics.ContactEventsTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_PreStep_FRAME, Statistics.PreStepTime);
		Unaccounted -= Statistics.PreStepTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_PostStep_FRAME, Statistics.PostStepTime);
		Unaccounted -= Statistics.PostStepTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_LastStep_FRAME, Statistics.LastStepTime);
		Unaccounted -= Statistics.LastStepTime;
		INC_FLOAT_STAT_BY(STAT_AGXD_Unaccounted_FRAME, Unaccounted);

		// Counters should not be incremented every step, only once per frame.
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

	const uint64 StartCycle = FPlatformTime::Cycles64();
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:UAGX_Simulation::Step"));
	int32 NumSteps = 0;
	switch (StepMode)
	{
		case SmCatchUpImmediately:
			NumSteps = StepCatchUpImmediately(DeltaTime);
			break;
		case SmCatchUpOverTime:
			NumSteps = StepCatchUpOverTime(DeltaTime);
			break;
		case SmCatchUpOverTimeCapped:
			NumSteps = StepCatchUpOverTimeCapped(DeltaTime);
			break;
		case SmDropImmediately:
			NumSteps = StepDropImmediately(DeltaTime);
			break;
		case SmNone:
			NumSteps = 0;
			break;
		default:
			UE_LOG(LogAGX, Error, TEXT("Unknown step mode: %d"), StepMode);
	}

	SET_DWORD_STAT(STAT_AGXU_NumSteps, NumSteps);

	// Unreal Engine will zero the stat counters every frame. If we can run the game loop faster
	// than the step-forward time then some frames won't do any stepping. That is, the above switch
	// will be a no-op and run in basically zero time. In those cases we want to report the time it
	// took to step the most recent frame we actually did some stepping in.
	if (NumSteps > 0)
	{
		// We did take a step, the time value is useful.
		const uint64 EndCycle = FPlatformTime::Cycles64();
		const double TotalStepTime = FPlatformTime::ToMilliseconds64(EndCycle - StartCycle);
		SET_FLOAT_STAT(STAT_AGXU_StepTime, TotalStepTime);
		LastTotalStepTime = TotalStepTime;
	}
	else
	{
		// We did not take a step, the time value is nearly zero and not useful.
		SET_FLOAT_STAT(STAT_AGXU_StepTime, LastTotalStepTime);
	}

	// Statistics will always hold the data for most recent Step Forward, so we can always report
	// it. Note that we do not call the frame accumulation version. This means that the frame timers
	// will be zero when no stepping was made. This makes it possible to see that no step was made
	// in a frame.
	if (bEnableStatistics)
	{
		FAGX_Statistics AGXStatistics = GetStatistics();
		ReportStepStatistics(AGXStatistics);
	}
}

int32 UAGX_Simulation::StepCatchUpImmediately(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	int32 NumSteps = 0;
	while (DeltaTime >= TimeStep)
	{
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
			NativeBarrier.Step();
		}
		++NumSteps;
		DeltaTime -= TimeStep;
		if (bEnableStatistics)
		{
			AGX_Simulation_helpers::AccumulateFrameStatistics(GetStatistics());
		}
	}
	LeftoverTime = DeltaTime;
	return NumSteps;
}

int32 UAGX_Simulation::StepCatchUpOverTime(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	int32 NumSteps = 0;
	// Step up to two times.
	for (int i = 0; i < 2; i++)
	{
		if (DeltaTime >= TimeStep)
		{
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
				NativeBarrier.Step();
			}
			++NumSteps;
			DeltaTime -= TimeStep;
			if (bEnableStatistics)
			{
				AGX_Simulation_helpers::AccumulateFrameStatistics(GetStatistics());
			}
		}
	}

	LeftoverTime = DeltaTime;
	return NumSteps;
}

int32 UAGX_Simulation::StepCatchUpOverTimeCapped(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	int32 NumSteps = 0;
	// Step up to two times.
	for (int i = 0; i < 2; i++)
	{
		if (DeltaTime >= TimeStep)
		{
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
				NativeBarrier.Step();
			}
			++NumSteps;
			DeltaTime -= TimeStep;
			if (bEnableStatistics)
			{
				AGX_Simulation_helpers::AccumulateFrameStatistics(GetStatistics());
			}
		}
	}

	// Cap the LeftoverTime according to the TimeLagCap.
	LeftoverTime = std::min(DeltaTime, TimeLagCap);
	return NumSteps;
}

int32 UAGX_Simulation::StepDropImmediately(float DeltaTime)
{
	DeltaTime += LeftoverTime;
	LeftoverTime = 0.0f;

	int32 NumSteps = 0;
	if (DeltaTime >= TimeStep)
	{
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
			NativeBarrier.Step();
		}
		++NumSteps;
		DeltaTime -= TimeStep;
		if (bEnableStatistics)
		{
			AGX_Simulation_helpers::AccumulateFrameStatistics(GetStatistics());
		}
	}

	// Keep LeftoverTime updated in case the information is needed in the future.
	LeftoverTime = DeltaTime;
	return NumSteps;
}

void UAGX_Simulation::StepOnce()
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

	const uint64 StartCycle = FPlatformTime::Cycles64();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:Native step"));
		NativeBarrier.Step();
	}
	INC_DWORD_STAT(STAT_AGXU_NumSteps);
	const uint64 EndCycle = FPlatformTime::Cycles64();
	const double StepTime = FPlatformTime::ToMilliseconds64(EndCycle - StartCycle);
	INC_FLOAT_STAT_BY(STAT_AGXU_StepTime, StepTime);
	if (bEnableStatistics)
	{
		FAGX_Statistics Statistics = GetStatistics();
		AccumulateFrameStatistics(Statistics);
		// We don't know if there are going to be more steps taken this frame or not, so we report
		// step statistics every time just in case.
		ReportStepStatistics(Statistics);
	}
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
	if (!GameInstance)
	{
		return nullptr;
	}

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
		return nullptr;
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
	void InvalidLicenseMessage(const FString& Status)
	{
		FString Message =
			"Invalid AGX Dynamics license. Status: " + Status +
			"\n\nIt will not be possible to run simulations using the AGX "
			"Dynamics for Unreal plugin.\n\nTo get your license, visit us at www.algoryx.se";

		if (!FAGX_Environment::IsSetupEnvRun())
		{
			const FString LicensePath = FAGX_Environment::GetPluginLicenseDirPath();
			Message += "\n\nThe AGX Dynamics license file should be placed in the directory: " +
					   LicensePath;
		}

#if defined(__linux__) && !WITH_EDITOR
		// On Linux, the message box sometimes end up behind the full-screen view in cooked builds.
		// Therefore we print the message directly to the screen for that case.
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 100.f, FColor::Red, Message);
		}
		UE_LOG(LogAGX, Error, TEXT("%s"), *Message);
#else
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Message);
#endif
	}
}

void UAGX_Simulation::EnsureValidLicense()
{
	FString Status;
	if (FAGX_Environment::GetInstance().EnsureAgxDynamicsLicenseValid(&Status) == false)
	{
		InvalidLicenseMessage(Status);
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
