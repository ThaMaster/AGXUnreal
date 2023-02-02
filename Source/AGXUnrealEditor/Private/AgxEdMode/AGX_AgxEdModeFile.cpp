// Copyright 2023, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeFile.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_EditorStyle.h"
#include "AGX_ArchiveExporter.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "Textures/SlateIcon.h"
#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeFile"

UAGX_AgxEdModeFile* UAGX_AgxEdModeFile::GetInstance()
{
	static UAGX_AgxEdModeFile* FileTool = nullptr;

	if (FileTool == nullptr)
	{
		FileTool = GetMutableDefault<UAGX_AgxEdModeFile>();
	}

	return FileTool;
}

void UAGX_AgxEdModeFile::ImportToBlueprint()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(
				NSLOCTEXT("AGX", "AGXUnrealImport", "Import AGX Dynamics archive or URDF"));

	TSharedRef<SAGX_ImportDialog> ImportDialog = SNew(SAGX_ImportDialog);
	Window->SetContent(ImportDialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);

	if (auto ImportSettings = ImportDialog->ToImportSettings())
	{
		AGX_ImporterToBlueprint::Import(*ImportSettings);
	}
}

void UAGX_AgxEdModeFile::ExportAgxArchive()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	if (Simulation == nullptr || !Simulation->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot export simulation contets to an AGX Dynamics archive because no "
				 "Simulation is "
				 "active. Launch a Play-In-Editor session."));

		FAGX_EditorUtilities::ShowNotification(LOCTEXT(
			"No sim available",
			"Cannot export simulation contets to an AGX Dynamics archive because no Simulation is "
			"active. Launch a Play-In-Editor session."));
		return;
	}

	FString Filename = FAGX_EditorUtilities::SelectNewFileDialog(
		"Select an AGX Archive to export", ".agx", "AGX Dynamics Archive|*.agx", "unreal.agx", "");

	if (Filename.IsEmpty())
	{
		return; // Logging done in FAGX_EditorUtilities::SelectNewFileDialog().
	}

	FString Extension = FPaths::GetExtension(Filename);
	if (Extension != "agx" && Extension != "aagx")
	{
		Filename += ".agx";
	}

	if (AGX_ArchiveExporter::ExportAGXArchive(Filename))
	{
		UE_LOG(LogAGX, Log, TEXT("AGX Dynamics archive saved to %s."), *Filename);
	}
	else
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX Dynamics archive could not be saved to %s."), *Filename);
	}
}

bool UAGX_AgxEdModeFile::SynchronizeModel_BP(UObject* Bp)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Bp);
	if (Blueprint == nullptr)
		return false;

	return FAGX_EditorUtilities::SynchronizeModel(*Blueprint);
}

FText UAGX_AgxEdModeFile::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "File");
}

FText UAGX_AgxEdModeFile::GetTooltip() const
{
	return LOCTEXT(
		"Tooltip",
		"Interoperability with external file formats, such AGX simulation files (.agx) "
		"or URDF (.urdf) files.");
}

FSlateIcon UAGX_AgxEdModeFile::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::FileIcon,
		FAGX_EditorStyle::FileIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
