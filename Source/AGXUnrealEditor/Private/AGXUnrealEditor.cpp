#include "AGXUnrealEditor.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#include "ImportAGXArchiveCommands.h"
#include "ImportAGXArchiveStyle.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"

// These includes are for what the buttom actually does.
#include "Engine/StaticMeshActor.h"
#include "Misc/MessageDialog.h"
#include "DesktopPlatformModule.h"

#include "AGXArchiveReader.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "AGX_ConstraintBodyAttachmentCustomization.h"
#include "Constraints/AGX_Constraint.h"

#include "AGX_TopMenu.h"

#include "RigidBodyBarrier.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"


void FAGXUnrealEditorModule::StartupModule()
{
	RegisterProjectSettings();
	RegisterCommands();
	RegisterCustomizations();

	AgxTopMenu = MakeShareable(new FAGX_TopMenu());
}

void FAGXUnrealEditorModule::ShutdownModule()
{
	UnregisterCommands();
	UnregisterProjectSettings();
	UnregisterCustomizations();

	AgxTopMenu = nullptr;
}

const TSharedPtr<FAGX_TopMenu>& FAGXUnrealEditorModule::GetAgxTopMenu() const
{
	return AgxTopMenu;
}

void FAGXUnrealEditorModule::RegisterProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "UAGX_Simulation",
			LOCTEXT("UAGX_Simulation_ProjectSettingsName", "AGX Physics"),
			LOCTEXT(
				"UAGX_Simulation_ProjectSettingsDesc", "Configure the simulation settings of the AGX Unreal plugin."),
			GetMutableDefault<UAGX_Simulation>());
	}
}

void FAGXUnrealEditorModule::UnregisterProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "UAGX_Simulation");
	}
}

void FAGXUnrealEditorModule::RegisterCommands()
{
	/// \todo Move the ImportAGXArchive button/menu entry somewhere AGXUnreal centric.
	FImportAGXArchiveStyle::Initialize();
	FImportAGXArchiveStyle::ReloadTextures();
	FImportAGXArchiveCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(FImportAGXArchiveCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FAGXUnrealEditorModule::PluginButtonClicked), FCanExecuteAction());
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands,
			FMenuExtensionDelegate::CreateRaw(this, &FAGXUnrealEditorModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this, &FAGXUnrealEditorModule::AddToolbarExtension));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FAGXUnrealEditorModule::UnregisterCommands()
{
	FImportAGXArchiveStyle::Shutdown();
	FImportAGXArchiveCommands::Unregister();
}

/// \todo Move Unreal Editor object creation from .agx to a dedicated class.
void FAGXUnrealEditorModule::PluginButtonClicked()
{
	FText DialogText = FText::Format(LOCTEXT("PluginButtonDialogText", "This is my new message: {0}."), FText::FromString(TEXT("MESSAGE")));
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog
	(
		nullptr, //const void * ParentWindowHandle,          /// \todo See https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html?sort=oldest
		TEXT("DialogTitle"), //const FString & DialogTitle,  ///       for a discussion
		TEXT("DefaultPath"), //const FString & DefaultPath,  ///       on window handles.
		TEXT("DefaultFile"), //const FString & DefaultFile,
		TEXT("AGX Dynamics Archive|*.agx"), //const FString & FileTypes,
		EFileDialogFlags::None,
		Filenames //TArray< FString > & OutFilenames
	);

	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No .agx file selected. Doing nothing"));
		return;
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(LogTemp, Log, TEXT("Multiple file selected but we only support single files for now. Doing nothing"));
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	const FString& Filename = Filenames[0];
	FAGXArchiveReader Archive(Filename);
	for (auto& BoxBody : Archive.GetBoxBodies())
	{
		/// \todo This is where the Unread Editor objects should be created.
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX box body at %f."), BoxBody.Body->GetPosition(World).X);
	}

	for (auto& SphereBody : Archive.GetSphereBodies())
	{
		/// \todo This is where the Unreal Editor objects should be created.
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX sphere body at %f."), SphereBody.Body->GetPosition(World).X);
	}
}

void FAGXUnrealEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FImportAGXArchiveCommands::Get().PluginAction);
}

void FAGXUnrealEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FImportAGXArchiveCommands::Get().PluginAction);
}

void FAGXUnrealEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAGX_ConstraintBodyAttachmentCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName());

	PropertyModule.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealEditorModule, AGXUnrealEditor);
