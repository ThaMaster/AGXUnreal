#include "AGXUnrealEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "IPlacementModeModule.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"

#include "Engine/StaticMeshActor.h"
#include "DesktopPlatformModule.h"

#include "AGX_ArchiveImporter.h"
#include "AGX_ArchiveExporter.h"
#include "AgxEdMode/AGX_AgxEdMode.h"
#include "AgxEdMode/AGX_AgxEdModeConstraints.h"
#include "AgxEdMode/AGX_AgxEdModeConstraintsCustomization.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "AgxEdMode/AGX_AgxEdModeFileCustomization.h"
#include "AGX_BoxShapeComponent.h"
#include "AGX_EditorStyle.h"
#include "AGX_EditorUtilities.h"
#include "AGX_LogCategory.h"
#include "AGX_MaterialManager.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_Simulation.h"
#include "AGX_TopMenu.h"
#include "Constraints/AGX_BallConstraint.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintBodyAttachmentCustomization.h"
#include "Constraints/AGX_ConstraintCustomization.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponentVisualizer.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"
#include "Constraints/AGX_ConstraintFrameComponentVisualizer.h"
#include "Constraints/AGX_CylindricalConstraint.h"
#include "Constraints/AGX_DistanceConstraint.h"
#include "Constraints/AGX_HingeConstraint.h"
#include "Constraints/AGX_LockConstraint.h"
#include "Constraints/AGX_PrismaticConstraint.h"
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
	RegisterModes();
	RegisterPlacementCategory();

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
	UnregisterModes();
	UnregisterPlacementCategory();

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
	// Nothing here yet.
}

void FAGXUnrealEditorModule::UnregisterCommands()
{
	// Nothing here yet.
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


void FAGXUnrealEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ConstraintBodyAttachmentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(AAGX_Constraint::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_ConstraintCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UAGX_AgxEdModeConstraints::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_AgxEdModeConstraintsCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UAGX_AgxEdModeFile::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_AgxEdModeFileCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(AAGX_Constraint::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_AgxEdModeConstraints::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_AgxEdModeFile::StaticClass()->GetFName());

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

void FAGXUnrealEditorModule::RegisterModes()
{
	FEditorModeRegistry::Get().RegisterMode<FAGX_AgxEdMode>(
		FAGX_AgxEdMode::EM_AGX_AgxEdModeId,
		LOCTEXT("AGX_AgxEdModeDisplayName", "AGX Dynamics Tools"),
		FSlateIcon(FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIcon, FAGX_EditorStyle::AgxIconSmall),
		/*bVisisble*/ true);
}

void FAGXUnrealEditorModule::UnregisterModes()
{
	FEditorModeRegistry::Get().UnregisterMode(FAGX_AgxEdMode::EM_AGX_AgxEdModeId);
}

void FAGXUnrealEditorModule::RegisterPlacementCategory()
{
	FPlacementCategoryInfo PlacementCategory(LOCTEXT("DisplayName", "AGX"), "AGX", TEXT("PMAGX"));
	IPlacementModeModule::Get().RegisterPlacementCategory(PlacementCategory);

	auto RegisterPlaceableItem = [&](UClass* Class)
	{
		IPlacementModeModule::Get().RegisterPlaceableItem(PlacementCategory.UniqueHandle, MakeShareable(
			new FPlaceableItem(nullptr, FAssetData(Class))));
	};

	RegisterPlaceableItem(AAGX_MaterialManager::StaticClass());
	RegisterPlaceableItem(AAGX_ConstraintFrameActor::StaticClass());
	RegisterPlaceableItem(AAGX_BallConstraint::StaticClass());
	RegisterPlaceableItem(AAGX_CylindricalConstraint::StaticClass());
	RegisterPlaceableItem(AAGX_DistanceConstraint::StaticClass());
	RegisterPlaceableItem(AAGX_HingeConstraint::StaticClass());
	RegisterPlaceableItem(AAGX_LockConstraint::StaticClass());
	RegisterPlaceableItem(AAGX_PrismaticConstraint::StaticClass());
}

void FAGXUnrealEditorModule::UnregisterPlacementCategory()
{
	if (IPlacementModeModule::IsAvailable())
	{
		IPlacementModeModule::Get().UnregisterPlacementCategory("AGX");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealEditorModule, AGXUnrealEditor);
