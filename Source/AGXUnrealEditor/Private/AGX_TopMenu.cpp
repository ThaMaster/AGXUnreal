#include "AGX_TopMenu.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "AGXUnrealEditor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_BallConstraintActor.h"
#include "Constraints/AGX_CylindricalConstraintActor.h"
#include "Constraints/AGX_DistanceConstraintActor.h"
#include "Constraints/AGX_HingeConstraintActor.h"
#include "Constraints/AGX_LockConstraintActor.h"
#include "Constraints/AGX_PrismaticConstraintActor.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FAGX_TopMenu"

FAGX_TopMenu::FAGX_TopMenu()
	: Extender(nullptr)
	, UnrealMenuBarExtension(nullptr)
{
	UE_LOG(LogAGX, Log, TEXT("FAGX_TopMenu::FAGX_TopMenu()"));

	// Get Prerequisites.

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> ExtensibilityManager =
		LevelEditorModule.GetMenuExtensibilityManager();

	// Create our Unreal Main Menu extender, and its callback delegate.

	Extender = MakeShareable(new FExtender());

	FMenuBarExtensionDelegate UnrealMenuBarExtensionDelegate =
		FMenuBarExtensionDelegate::CreateStatic(&FAGX_TopMenu::CreateTopMenu);

	UnrealMenuBarExtension = Extender->AddMenuBarExtension(
		"Help", EExtensionHook::Before,
		nullptr, // Using inline FActions instead of FUICommands, for less hot reloading problems!
		UnrealMenuBarExtensionDelegate); // Delegate is only invoked during Editor startup (i.e.
										 // when Unreal Main Menu Bar is built).

	ExtensibilityManager->AddExtender(Extender);
}

FAGX_TopMenu::~FAGX_TopMenu()
{
	UE_LOG(LogAGX, Log, TEXT("FAGX_TopMenu::~FAGX_TopMenu()"));

	// Get prerequisites.

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> ExtensibilityManager =
		LevelEditorModule.GetMenuExtensibilityManager();

	// Cleanup.

	if (Extender)
	{
		if (UnrealMenuBarExtension)
			Extender->RemoveExtension(UnrealMenuBarExtension.ToSharedRef());

		if (ExtensibilityManager.IsValid())
			ExtensibilityManager->RemoveExtender(Extender);
	}

	UnrealMenuBarExtension = nullptr;
	Extender = nullptr;
}

/*static*/ void FAGX_TopMenu::CreateTopMenu(FMenuBarBuilder& Builder)
{
	// The reason why we use the complicated approach below is because of the
	// following problem in Unreal:
	//
	// If this module is recompiled and reloaded without restarting the Editor,
	// the old Unreal Main Menu Bar extension, its callback delegate, the AGX Top Menu
	// (header of the pull down menu only), and its callback delegate (for filling it,
	// see below) seem to still be in use. This is because Unreal only rebuilds its
	// Main Menu Bar during Editor startup (as it seems).
	//
	// Our first loaded Main Menu Bar extension callback delegate is invoked once, during
	// Editor startup, and then never again, even if the module is reloaded. And the
	// AGX Top Menu delegate callback is invoked each time AGX Top Menu is clicked, but with
	// the first loaded delegate, which is incorrect if we have recompiled our module.
	//
	// So, without our hack below, after module recompile the old AGX Top Menu delegate will
	// still point to a member function in the old instance of FAGX_TopMenu, which will quickly
	// lead to crashes. Furthermore, static functions such as FAGX_TopMenuCommands::Get, which
	// are frequently used in the delegate implementations, will be executed on static objects
	// in the old dll module (which still exists, Unreal just adds a new one and "abandons"
	// the old one).
	//
	// To work around this, we need to make sure that callbacks are:
	// 1. Using correct instance of FAGX_TopMenu (i.e. the this pointer), and
	// 2. Are executed in the context of the new dll module so that correct static objects are used.
	//
	// Hacky solution:
	//
	// 1. Is solved by fetching the new correct instance of FAGX_TopMenu from
	// FAGXUnrealEditorModule, and invoke our target function (FillTopMenu) on that instance.
	//
	// 2. Is solved by making our target function (FillTopMenu) virtual so that the v-table picks
	// the implementation of the function that exists in the new dll module, and therefore is
	// exectued in the next dll module context.
	//
	// Limitations:
	//
	// Can only make changes to FillTopMenu and sub-menues without resterating the Editor,
	// (e.g. cannot change name or position of the AGX Top Menu).
	//

	FNewMenuDelegate NewMenuDelegate = FNewMenuDelegate::CreateLambda([](FMenuBuilder& Builder) {
		UE_LOG(LogAGX, Log, TEXT("NewMenuDelegate"));

		if (FAGXUnrealEditorModule* AGXUnrealEditorModule =
				FModuleManager::GetModulePtr<FAGXUnrealEditorModule>("AGXUnrealEditor"))
		{
			if (TSharedPtr<FAGX_TopMenu> AgxTopMenu = AGXUnrealEditorModule->GetAgxTopMenu())
			{
				UE_LOG(LogAGX, Log, TEXT("&AgxTopMenu = %p"), AgxTopMenu.Get());

				AgxTopMenu->FillTopMenu(Builder);
			}
		}
	});

	Builder.AddPullDownMenu(
		LOCTEXT("TopMenuLabel", "AGX"), LOCTEXT("TopMenuToolTip", "Open the AGX top menu"),
		NewMenuDelegate); // Delegate is invoked when AGX menu is clicked (but same delegate
						  // regardless of reloading this module).
}

/*virtual*/ void FAGX_TopMenu::FillTopMenu(FMenuBuilder& Builder)
{
	Builder.AddSubMenu(
		LOCTEXT("FileMenuLabel", "File"),
		LOCTEXT(
			"FileMenuTooltip",
			"Interoperability with external file formats, such AGX simulation files (.agx)."),
		FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillFileMenu));

	Builder.AddMenuSeparator();

	Builder.AddSubMenu(
		LOCTEXT("ConstraintMenuLabel", "Constraints"),
		LOCTEXT("ConstraintMenuTooltip", "Create a constraint."),
		FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillConstraintMenu));

	Builder.AddMenuSeparator();

	Builder.AddMenuEntry(
		LOCTEXT("AboutAgxDialogLabel", "About AGX Unreal..."),
		LOCTEXT("AboutAgxDialogToolTip", "Open the About AGX Window."), FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnOpenAboutDialogClicked), NAME_None,
		EUserInterfaceActionType::Button);
}

void FAGX_TopMenu::FillConstraintMenu(FMenuBuilder& Builder)
{
	AddFileMenuEntry(
		Builder, LOCTEXT("CreateBallConstraintLabel", "Create Ball Constraint"),
		LOCTEXT(
			"CreateBallConstraintTooltip",
			"Create Ball Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_BallConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateCylindricalConstraintLabel", "Create Cylindrical Constraint"),
		LOCTEXT(
			"CreateCylindricalConstraintTooltip",
			"Create Cylindrical Constraint. \n\nInitially setup using currently selected Rigid "
			"Body Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_CylindricalConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateDistanceConstraintLabel", "Create Distance Constraint"),
		LOCTEXT(
			"CreateDistanceConstraintTooltip",
			"Create Distance Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_DistanceConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateHingeConstraintLabel", "Create Hinge Constraint"),
		LOCTEXT(
			"CreateHingeConstraintTooltip",
			"Create Hinge Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_HingeConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateLockConstraintLabel", "Create Lock Constraint"),
		LOCTEXT(
			"CreateLockConstraintTooltip",
			"Create Lock Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_LockConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreatePrismaticConstraintLabel", "Create Prismatic Constraint"),
		LOCTEXT(
			"CreatePrismaticConstraintTooltip",
			"Create Prismatic Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_PrismaticConstraintActor::StaticClass());
		});
}

void FAGX_TopMenu::FillFileMenu(FMenuBuilder& Builder)
{
	// Import AGX Dynamics archive to in-level single actor menu item.
	AddFileMenuEntry(
			Builder,
			LOCTEXT(
					"FileMenuEntryLabelImportSingleActor",
					"Import AGX Dynamics archive to level as a single actor..."),
			LOCTEXT(
					"FileMenuEntryTooTipImportSingleActor",
					"Import an AGX Dynamics archive into the current level as a single actor"),
			[]() { UAGX_AgxEdModeFile::ImportAgxArchiveToSingleActor(); });

	// Import AGX archive to in-level actor tree menu item.
	AddFileMenuEntry(
		Builder,
		LOCTEXT(
			"FileMenuEntryLabelImportActorTree",
			"Import AGX Dynamics archive to level as an actor tree..."),
		LOCTEXT(
			"FileMenuEntryToolTipImportActorTree",
			"Import an AGX Dynamics Archive into the current level as a tree of actors."),
		[]() { UAGX_AgxEdModeFile::ImportAgxArchiveToActorTree(); });

	// Import AGX Dynamics archive to blueprint menu item.
	AddFileMenuEntry(
		Builder,
		LOCTEXT(
			"FileMEnuEntryLabelImportBluePrint", "Import AGX Dynamics archive to a Blueprint..."),
		LOCTEXT(
			"FileMenuEntryhTooltopImportBluePrint",
			"Import an AGX Dynamics archive to a Blueprint."),
		[]() { UAGX_AgxEdModeFile::ImportAgxArchiveToBlueprint(); });

	// Export AGX Archive menu item
	AddFileMenuEntry(
		Builder, LOCTEXT("FileMenuEntryLabelEx", "Export AGX Archive..."),
		LOCTEXT("FileMenuEntryToolTipEx", "Export an AGX Archive from the Editor."),
		[]() { UAGX_AgxEdModeFile::ExportAgxArchive(); });
}

void FAGX_TopMenu::OnCreateConstraintClicked(UClass* ConstraintClass)
{
	AActor* Actor1 = nullptr;
	AActor* Actor2 = nullptr;
	FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
		&Actor1, &Actor2,
		/*bSearchSubtrees*/ true, /*bSearchAncestors*/ true);

	if (Actor1 == nullptr || Actor2 == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Must select two actors before creating a constraint."));
		return;
	}

	/// \todo Figure out how to setup constraint creation so that we can pick a
	/// single UAGX_RigidBodyComponent from the selected Actors. There is very
	/// similar code in AGX_AgxEdModeConstraints.cpp.

	TArray<UAGX_RigidBodyComponent*> Bodies1 = UAGX_RigidBodyComponent::GetFromActor(Actor1);
	TArray<UAGX_RigidBodyComponent*> Bodies2 = UAGX_RigidBodyComponent::GetFromActor(Actor2);

	if (Bodies1.Num() != 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create constraint with actor '%s' because it doesn't contain exactly one "
				 "body."),
			*Actor1->GetName());
		return;
	}

	if (Bodies2.Num() != 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create constraint with actor '%s' because it doesn't contain exactly one "
				 "body."),
			*Actor2->GetName());
		return;
	}

	AAGX_ConstraintActor* Constraint = FAGX_EditorUtilities::CreateConstraintActor(
		ConstraintClass, Bodies1[0], Bodies2[0],
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);

	UAGX_ConstraintComponent* ConstraintComponent = Constraint->GetConstraintComponent();
	ConstraintComponent->BodyAttachment1.FrameDefiningSource = EAGX_FrameDefiningSource::CONSTRAINT;
	ConstraintComponent->BodyAttachment2.FrameDefiningSource = EAGX_FrameDefiningSource::CONSTRAINT;
	ConstraintComponent->BodyAttachment1.OnFremeDefiningSourceChanged();
	ConstraintComponent->BodyAttachment2.OnFremeDefiningSourceChanged();
}

void FAGX_TopMenu::OnOpenAboutDialogClicked()
{
	FText Title = LOCTEXT("AboutDialogTitle", "About AGX Unreal");

	FText Message = LOCTEXT(
		"AboutDialogMessage",
		"\n"
		"Copyright: Algoryx Simulation AB\n"
		"\n"
		"Plugin version: N/A\n"
		"AGX Dynamics version: N/A\n"
		"\n"
		"License valid until: N/A\n"
		"Licensee: N/A");

	FMessageDialog::Open(EAppMsgType::Ok, Message, &Title);
}

#undef LOCTEXT_NAMESPACE
