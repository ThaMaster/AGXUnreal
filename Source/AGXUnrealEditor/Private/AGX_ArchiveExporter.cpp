// Copyright 2022, Algoryx Simulation AB.


#include "AGX_ArchiveExporter.h"

#include "Utilities/AGX_EditorUtilities.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

#include "Engine/World.h"

bool AGX_ArchiveExporter::ExportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The current world does not have a simulation. Cannot store AGX Dynamics "
				 "archive."));
		return false;
	}
	return Simulation->WriteAGXArchive(ArchivePath);
}
