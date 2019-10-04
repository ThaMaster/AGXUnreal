#include "AGXUnrealEditor.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#include "AGX_Simulation.h"
#include "Constraints/AGX_Constraint.h"


#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

void FAGXUnrealEditorModule::StartupModule()
{
	RegisterProjectSettings();
	RegisterCustomizations();
}

void FAGXUnrealEditorModule::ShutdownModule()
{
	UnregisterProjectSettings();
	UnregisterCustomizations();
}

void FAGXUnrealEditorModule::RegisterProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", "Plugins", "UAGX_Simulation",
			LOCTEXT("UAGX_Simulation_ProjectSettingsName", "AGX Physics"),
			LOCTEXT("UAGX_Simulation_ProjectSettingsDesc", "Configure the simulation settings of the AGX Unreal plugin."),
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