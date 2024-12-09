// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_Importer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/AnyConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Import/AGX_ImporterSettings.h"
#include "Import/AGXSimObjectsReader.h"
#include "Import/SimulationObjectCollection.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Vehicle/TrackBarrier.h"

namespace AGX_Importer_helpers
{
	bool IsUnrecoverableError(EAGX_ImportInstantiationResult Result)
	{
		return Result == EAGX_ImportInstantiationResult::FatalError ||
				Result == EAGX_ImportInstantiationResult::Invalid;
	}

	AActor* CreateActor(const FString& Name)
	{
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to create an Actor during import, got an invalid empty name."));
			return nullptr;
		}

		AActor* NewActor = NewObject<AActor>();
		if (!NewActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create new actor during import."));
			return nullptr;
		}

		NewActor->Rename(*Name);
		return NewActor;
	}

	bool CreateSimulationObjectCollection(
		const FString& FilePath, FSimulationObjectCollection& OutSimObjects)
	{
		if (!FAGXSimObjectsReader::ReadAGXArchive(FilePath, OutSimObjects))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to import file '%s'. Log category LogAGX in the "
					 "Output Log may contain more information."),
				*FilePath);
			return false;
		}

		return true;
	}

	FString GetModelName(const FString& FilePath)
	{
		// Todo: sanitize name.
		FString Name = FPaths::GetBaseFilename(FilePath);
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Unable to generate a valid model name from file '%s'."),
				*FilePath);
			return "";
		}

		return Name;
	}

	EAGX_ImportInstantiationResult AddComponents(
		const FSimulationObjectCollection& SimObjects, const FAGX_ImporterSettings& Settings,
		AActor& OutActor)
	{
		for (const auto& Body : SimObjects.GetRigidBodies())
		{

		}
		return EAGX_ImportInstantiationResult::Success;
	}
}

FAGX_ImportResult FAGX_Importer::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_Importer_helpers;

	AActor* Actor = CreateActor(GetModelName(Settings.FilePath));
	if (Actor == nullptr)
		return FAGX_ImportResult(EAGX_ImportInstantiationResult::FatalError);

	FSimulationObjectCollection SimObjects;
	if (!CreateSimulationObjectCollection(Settings.FilePath, SimObjects))
		return FAGX_ImportResult(EAGX_ImportInstantiationResult::FatalError);

	EAGX_ImportInstantiationResult Result = AddComponents(SimObjects, Settings, *Actor);
	if (IsUnrecoverableError(Result))
		return FAGX_ImportResult(Result);

	return FAGX_ImportResult(Result, Actor);
}
