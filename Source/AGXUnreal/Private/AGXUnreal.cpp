#include "AGXUnreal.h"

#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"
#include "AGX_CollisionGroupManager.h"
#include "AGX_CollisionGroupManagerCustomize.h"
#include "AGX_CollisionGroups.h"
#include "AGX_CollisionGroupsCustomization.h"

#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealModule"

void FAGXUnrealModule::StartupModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::StartupModule()"));

	FAGX_RuntimeStyle::Initialize();
	FAGX_RuntimeStyle::ReloadTextures();

	RegisterCustomizations();
}

void FAGXUnrealModule::ShutdownModule()
{
	UE_LOG(LogAGX, Log, TEXT("FAGXUnrealModule::ShutdownModule()"));
	FAGX_RuntimeStyle::Shutdown();

	UnregisterCustomizations();
}

void FAGXUnrealModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		AAGX_CollisionGroupManager::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_CollisionGroupManagerCustomize::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_CollisionGroups::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_CollisionGroupsCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		AAGX_CollisionGroupManager::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		UAGX_CollisionGroups::StaticClass()->GetFName());

	PropertyModule.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealModule, AGXUnreal)
