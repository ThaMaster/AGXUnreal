#include "AgxEdMode/AGX_AgxEdModeFile.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"
#include "AGX_ArchiveImporterToSingleActor.h"
#include "AGX_ArchiveImporterToBlueprint.h"
#include "AGX_ArchiveExporter.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "Textures/SlateIcon.h"
#include "DesktopPlatformModule.h"
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

namespace
{
	static const FString NONE_SELECTED("");

	FString SelectExistingAgxArchive()
	{
		// For a discussion on window handles see
		// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
		TArray<FString> Filenames;
		bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(
			nullptr, TEXT("Select an AGX Archive to import"), TEXT("DefaultPath"),
			TEXT("DefaultFile"), TEXT("AGX Dynamics Archive|*.agx"), EFileDialogFlags::None,
			Filenames);
		if (!FileSelected || Filenames.Num() == 0)
		{
			UE_LOG(LogAGX, Log, TEXT("No .agx file selected. Doing nothing."));
			return NONE_SELECTED;
		}
		if (Filenames.Num() > 1)
		{
			UE_LOG(
				LogAGX, Log,
				TEXT(
					"Multiple files selected but we only support single file import for now. Doing "
					"nothing."));
			FAGX_EditorUtilities::ShowNotification(LOCTEXT(
				"Multiple .agx",
				"Multiple files selected but we only support single files for now. Doing "
				"nothing."));
			return NONE_SELECTED;
		}
		FString Filename = Filenames[0];
		return Filename;
	}
}

void UAGX_AgxEdModeFile::ImportAgxArchiveToSingleActor()
{
	const FString Filename = SelectExistingAgxArchive();
	if (Filename == NONE_SELECTED)
	{
		return;
	}
	AGX_ArchiveImporterToSingleActor::ImportAGXArchive(Filename);
}

void UAGX_AgxEdModeFile::ImportAgxArchiveToBlueprint()
{
	const FString Filename = SelectExistingAgxArchive();
	if (Filename == NONE_SELECTED)
	{
		return;
	}

	AGX_ArchiveImporterToBlueprint::ImportAGXArchive(Filename);
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

	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->SaveFileDialog(
		nullptr, TEXT("Select an AGX Archive to export"), TEXT(""), TEXT("unreal.agx"),
		TEXT("AGX Dynamics Archive|*.agx"), EFileDialogFlags::None, Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("No .agx file selected, Doing nothing."));
		return;
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Multiple files selected but we only support exporting to one. Doing nothing."));
		FAGX_EditorUtilities::ShowNotification(LOCTEXT(
			"Multiple .agx export",
			"Multiple files selected but we only support exporting to one. Doing nothing."));
		return;
	}

	FString Filename = Filenames[0];
	if (Filename.IsEmpty())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot store AGX Dynamics archive to an empty file name. Doing nothing."));
		FAGX_EditorUtilities::ShowNotification(LOCTEXT(
			"Empty .agx name export",
			"Cannot store AGX Dynamics archive to an empty file name. Doing nothing."));
		return;
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

FText UAGX_AgxEdModeFile::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "File");
}

FText UAGX_AgxEdModeFile::GetTooltip() const
{
	return LOCTEXT(
		"Tooltip", "Interoperability with external file formats, such AGX simulation files (.agx)");
}

FSlateIcon UAGX_AgxEdModeFile::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIcon,
		FAGX_EditorStyle::AgxIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
