#include "AGXUnrealEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#include "ImportAGXArchiveCommands.h"
#include "ImportAGXArchiveStyle.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"

#include "Engine/StaticMeshActor.h"
#include "DesktopPlatformModule.h"

#include "AGX_ArchiveImporter.h"
#include "AGX_BoxShapeComponent.h"
#include "AGX_EditorStyle.h"
#include "AGX_EditorUtilities.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_Simulation.h"
#include "AGX_TopMenu.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintBodyAttachmentCustomization.h"
#include "Constraints/AGX_ConstraintCustomization.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponentVisualizer.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"
#include "Constraints/AGX_ConstraintFrameComponentVisualizer.h"
#include "Materials/AGX_ContactMaterialAssetTypeActions.h"
#include "Materials/AGX_MaterialAssetTypeActions.h"
#include "RigidBodyBarrier.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

void FAGXUnrealEditorModule::StartupModule()
{
	FAGX_EditorStyle::Initialize();
	FAGX_EditorStyle::ReloadTextures();

	RegisterProjectSettings();
	RegisterCommands();
	RegisterAssetTypeActions();
	RegisterCustomizations();
	RegisterComponentVisualizers();

	AgxTopMenu = MakeShareable(new FAGX_TopMenu());
}

void FAGXUnrealEditorModule::ShutdownModule()
{
	FAGX_EditorStyle::Shutdown();

	UnregisterCommands();
	UnregisterProjectSettings();
	UnregisterAssetTypeActions();
	UnregisterCustomizations();
	UnregisterComponentVisualizers();

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
			LOCTEXT("UAGX_Simulation_ProjectSettingsName", "AGX Dynamics"),
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

void FAGXUnrealEditorModule::RegisterAssetTypeActions()
{
	IAssetTools &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	EAssetTypeCategories::Type AgxAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("AgxUnreal")), LOCTEXT("AgxAssetCategory", "AGX"));

	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAGX_ContactMaterialAssetTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAGX_MaterialAssetTypeActions(AgxAssetCategoryBit)));
}

void FAGXUnrealEditorModule::UnregisterAssetTypeActions()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
		return;

	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

	for (const TSharedPtr<IAssetTypeActions>& AssetTypeAction : RegisteredAssetTypeActions)
	{
		AssetTools.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
	}
}

void FAGXUnrealEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedPtr<IAssetTypeActions>& Action)
{
	AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
	RegisteredAssetTypeActions.Add(Action);
}

void FAGXUnrealEditorModule::PluginButtonClicked()
{
	/// \todo See
	/// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html?sort=oldest
	/// for a discussion on window handles.
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(nullptr, TEXT("DialogTitle"), TEXT("DefaultPath"),
		TEXT("DefaultFile"), TEXT("AGX Dynamics Archive|*.agx"), EFileDialogFlags::None, Filenames);

	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No .agx file selected. Doing nothing"));
		return;
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(LogTemp, Log, TEXT("Multiple file selected but we only support single files for now. Doing nothing"));
		FAGX_EditorUtilities::ShowNotification(
			LOCTEXT("Multiple .agx", "Multiple file selected but we only support single files for now. Doing nothing"));
		return;
	}

	FString Filename = Filenames[0];
	AGX_ArchiveImporter::ImportAGXArchive(Filename);
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

	PropertyModule.RegisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ConstraintBodyAttachmentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(AAGX_Constraint::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_ConstraintCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(AAGX_Constraint::StaticClass()->GetFName());

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::RegisterComponentVisualizers()
{
	RegisterComponentVisualizer(
		UAGX_ConstraintComponent::StaticClass()->GetFName(), MakeShareable(new FAGX_ConstraintComponentVisualizer));
	RegisterComponentVisualizer(UAGX_ConstraintFrameComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_ConstraintFrameComponentVisualizer));
}

void FAGXUnrealEditorModule::UnregisterComponentVisualizers()
{
	UnregisterComponentVisualizer(UAGX_ConstraintComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_ConstraintFrameComponent::StaticClass()->GetFName());
}

void FAGXUnrealEditorModule::RegisterComponentVisualizer(
	const FName& ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

void FAGXUnrealEditorModule::UnregisterComponentVisualizer(const FName& ComponentClassName)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(ComponentClassName);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealEditorModule, AGXUnrealEditor);
