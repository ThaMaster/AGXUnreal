#include "AgxEdMode/AGX_AgxEdModeFile.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToSingleActor.h"
#include "AGX_ImporterToActorTree.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ArchiveExporter.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
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

namespace
{
	static const FString NONE_SELECTED("");

	FString SelectExistingFile(const FString& FileDescription, const FString& FileExtension)
	{
		const FString DialogTitle = FString("Select an ") + FileDescription + FString(" to import");
		const FString FileTypes = FileDescription + FString("|*") + FileExtension;
		// For a discussion on window handles see
		// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
		TArray<FString> Filenames;
		bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(
			nullptr, DialogTitle, TEXT("DefaultPath"), TEXT("DefaultFile"), FileTypes,
			EFileDialogFlags::None, Filenames);
		if (!FileSelected || Filenames.Num() == 0)
		{
			UE_LOG(LogAGX, Log, TEXT("No %s file selected. Doing nothing."), *FileExtension);
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
				"Multiple files",
				"Multiple files selected but we only support single files for now. Doing "
				"nothing."));
			return NONE_SELECTED;
		}
		FString Filename = Filenames[0];
		return Filename;
	}

	FString SelectExistingDirectory(const FString& DialogTitle, bool AllowNoneSelected = false)
	{
		// For a discussion on window handles see
		// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
		FString DirectoryPath("");
		bool DirectorySelected = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
			nullptr, DialogTitle, TEXT("DefaultPath"), DirectoryPath);

		if (!AllowNoneSelected && (!DirectorySelected || DirectoryPath.IsEmpty()))
		{
			UE_LOG(LogAGX, Log, TEXT("No directory selected. Doing nothing."));
			return NONE_SELECTED;
		}

		return DirectoryPath;
	}

	bool UrdfHasFilenameAttribute(const FString& FilePath)
	{
		FString Content;
		if(!FFileHelper::LoadFileToString(Content, *FilePath))
		{
			UE_LOG(LogAGX, Warning, TEXT("Unable to read file '%s'"), *FilePath);
			return false;
		}

		return Content.Contains("filename", ESearchCase::IgnoreCase);
	}
}

void UAGX_AgxEdModeFile::ImportAgxArchiveToSingleActor()
{
	const FString Filename = SelectExistingFile("AGX Dynamics Archive", ".agx");
	if (Filename == NONE_SELECTED)
	{
		return;
	}
	AGX_ImporterToSingleActor::ImportAGXArchive(Filename);
}

void UAGX_AgxEdModeFile::ImportAgxArchiveToActorTree()
{
	const FString Filename = SelectExistingFile("AGX Dynamics Archive", ".agx");
	if (Filename == NONE_SELECTED)
	{
		return;
	}
	AGX_ImporterToActorTree::ImportAGXArchive(Filename);
}

void UAGX_AgxEdModeFile::ImportAgxArchiveToBlueprint()
{
	const FString Filename = SelectExistingFile("AGX Dynamics Archive", ".agx");
	if (Filename == NONE_SELECTED)
	{
		return;
	}

	AGX_ImporterToBlueprint::ImportAGXArchive(Filename);
}

void UAGX_AgxEdModeFile::ImportUrdfToBlueprint()
{
	const FString UrdfFilePath = SelectExistingFile("URDF file", ".urdf");
	if (UrdfFilePath == NONE_SELECTED)
	{
		return;
	}

	const FString UrdfPackagePath = [&UrdfFilePath]()
	{
		if (UrdfHasFilenameAttribute(UrdfFilePath))
		{
			return SelectExistingDirectory("(Optional) Select URDF package directory", true);
		}
		else
		{
			return FString();
		}
	}();


	AGX_ImporterToBlueprint::ImportURDF(UrdfFilePath, UrdfPackagePath);
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
		"Tooltip",
		"Interoperability with external file formats, such AGX simulation files (.agx) "
		"or URDF (.urdf) files.");
}

#undef LOCTEXT_NAMESPACE
