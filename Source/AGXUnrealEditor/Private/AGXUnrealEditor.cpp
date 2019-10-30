#include "AGXUnrealEditor.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#include "ImportAGXArchiveCommands.h"
#include "ImportAGXArchiveStyle.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"

#include "Engine/StaticMeshActor.h"
#include "DesktopPlatformModule.h"

#include "AGXArchiveReader.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "AGX_ConstraintBodyAttachmentCustomization.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_BoxShapeComponent.h"
#include "Constraints/AGX_Constraint.h"
#include "AGX_EditorUtilities.h"

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

	UClass* ActorClass = AActor::StaticClass();
	FName RootName = USceneComponent::GetDefaultSceneRootVariableName();
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	check(World);

	FAGXArchiveReader Archive(Filenames[0]);
	/// \todo Proper error handling.
	if (Archive.GetBoxBodies().Num() == 0 && Archive.GetSphereBodies().Num() == 0)
	{
		FAGX_EditorUtilities::ShowNotification(
			LOCTEXT("No bodies", "No bodies found in .agx archive. Perhaps read failure."));
		return;
	}
	for (auto& BoxBody : Archive.GetBoxBodies())
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX box body with name %s at %f."), *BoxBody.Body->GetName(),
			BoxBody.Body->GetPosition(World).X);

		const FRigidBodyBarrier* Body = BoxBody.Body;
		const FBoxShapeBarrier* Box = BoxBody.Box;

		/// \todo Consider using the state synchronization functions we already
		/// have, the ones used between time steps.

		FTransform Transform(Body->GetRotation(), Body->GetPosition(World));
		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Transform);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s."), *Body->GetName());
			continue;
		}
		NewActor->SetActorLabel(Body->GetName());

		/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
		/// Related to undo/redo, I think.
		USceneComponent* Root = NewObject<USceneComponent>(NewActor, RootName /*, RF_Transactional*/);
		NewActor->SetRootComponent(Root);
		NewActor->AddInstanceComponent(Root);
		Root->RegisterComponent();

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		if (NewBody == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create AGX RigidBody for %s."), *Body->GetName());
			continue;
		}
		NewBody->Rename(TEXT("UAGX_RigidBody"));
		NewBody->Mass = Body->GetMass();
		NewBody->MotionControl = Body->GetMotionControl();

		UAGX_BoxShapeComponent* NewBox = FAGX_EditorUtilities::CreateBoxShape(NewActor, Root);
		NewBox->HalfExtent = Box->GetHalfExtents(World);
	}

	for (auto& SphereBody : Archive.GetSphereBodies())
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX sphere body with name %s at %f."), *SphereBody.Body->GetName(),
			SphereBody.Body->GetPosition(World).X);

		const FRigidBodyBarrier* Body = SphereBody.Body;
		const FSphereShapeBarrier* Sphere = SphereBody.Sphere;

		/// \todo Consider using the state synchronization functions we already
		/// have, the ones used between time steps.

		FTransform Transform(Body->GetRotation(), Body->GetPosition(World));
		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Transform);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s'."), *Body->GetName());
			continue;
		}
		NewActor->SetActorLabel(Body->GetName());

		/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
		/// Related to undo/redo, I think.
		USceneComponent* Root = NewObject<USceneComponent>(NewActor, RootName /*, RF_Transactional*/);
		NewActor->SetRootComponent(Root);
		NewActor->AddInstanceComponent(Root);
		Root->RegisterComponent();

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		NewBody->Rename(TEXT("AGX_RigidBody"));
		NewBody->Mass = Body->GetMass();
		NewBody->MotionControl = Body->GetMotionControl();

		UAGX_SphereShapeComponent* NewSphere = FAGX_EditorUtilities::CreateSphereShape(NewActor, Root);
		NewSphere->Radius = Sphere->GetRadius(World);
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

	PropertyModule.RegisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ConstraintBodyAttachmentCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.UnregisterCustomPropertyTypeLayout(FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName());

	PropertyModule.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealEditorModule, AGXUnrealEditor);
