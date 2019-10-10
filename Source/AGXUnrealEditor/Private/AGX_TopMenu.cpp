#include "AGX_TopMenu.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#include "AGX_LogCategory.h"
#include "Constraints/AGX_CylindricalConstraint.h"
#include "Constraints/AGX_DistanceConstraint.h"
#include "Constraints/AGX_HingeConstraint.h"
#include "Constraints/AGX_LockConstraint.h"
#include "Constraints/AGX_PrismaticConstraint.h"

#include "AGXUnrealEditor.h"
#include "AGX_EditorUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TopMenu"


FAGX_TopMenu::FAGX_TopMenu()
	:
Extender(nullptr),
UnrealMenuBarExtension(nullptr)
{
	UE_LOG(LogTemp, Log, TEXT("FAGX_TopMenu::FAGX_TopMenu()"));

	
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
		nullptr, // Using inlined FActions instead of FUICommands, for less hot reloading problems!
		UnrealMenuBarExtensionDelegate); // Delegate is only invoked during Editor startup (i.e. when Unreal Main Menu Bar is built).
		
	ExtensibilityManager->AddExtender(Extender);
}


FAGX_TopMenu::~FAGX_TopMenu()
{
	UE_LOG(LogTemp, Log, TEXT("FAGX_TopMenu::~FAGX_TopMenu()"));

	// Get prerequisites.

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> ExtensibilityManager =
		LevelEditorModule.GetMenuExtensibilityManager();
	

	// Cleanup.
	
	if (Extender)
	{
		if(UnrealMenuBarExtension)
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
	// (header of the pulldown menu only), and its callback delegate (for filling it,
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
	// 1. Is solved by fetching the new correct instance of FAGX_TopMenu from FAGXUnrealEditorModule,
	// and invoke our target function (FillTopMenu) on that instance.
	//
	// 2. Is solved by making our target function (FillTopMenu) virtual so that the v-table picks
	// the implementation of the function that exists in the new dll module, and therefore is exectued
	// in the next dll module context.
	//
	// Limitations:
	// 
	// Can only make changes to FillTopMenu and sub-menues without resterating the Editor,
	// (e.g. cannot change name or position of the AGX Top Menu).
	// 

	FNewMenuDelegate NewMenuDelegate =
		FNewMenuDelegate::CreateLambda([](FMenuBuilder& Builder)
	{
		UE_LOG(LogTemp, Log, TEXT("NewMenuDelegate"));

		if (FAGXUnrealEditorModule* AGXUnrealEditorModule =
			FModuleManager::GetModulePtr<FAGXUnrealEditorModule>("AGXUnrealEditor"))
		{
			if (TSharedPtr<FAGX_TopMenu> AgxTopMenu = AGXUnrealEditorModule->GetAgxTopMenu())
			{
				UE_LOG(LogTemp, Log, TEXT("&AgxTopMenu = %p"), AgxTopMenu.Get());

				AgxTopMenu->FillTopMenu(Builder);
			}
		}
	});

	Builder.AddPullDownMenu(
		LOCTEXT("TopMenuLabel", "AGX"),
		LOCTEXT("TopMenuToolTip", "Open the AGX top menu"),
		NewMenuDelegate);  // Delegate is invoked when AGX menu is clicked (but same delegate regardless of reloading this module).
}


/*virtual*/ void FAGX_TopMenu::FillTopMenu(FMenuBuilder& Builder)
{
	Builder.AddSubMenu(
		LOCTEXT("ConstraintMenuLabel", "Constraints"),
		LOCTEXT("ConstraintMenuTooltip", "Create a constraint"),
		FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillConstraintMenu));

	Builder.AddMenuSeparator();

	Builder.AddMenuEntry(
		LOCTEXT("AboutAgxDialogLabel", "About AGX Unreal..."),
		LOCTEXT("AboutAgxDialogToolTip", "Open the About AGX Window"),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnOpenAboutDialogClicked),
		NAME_None,
		EUserInterfaceActionType::Button);
}


#define ADD_CREATE_CONSTRAINT_MENU_ENTRY(ConstraintType, ConstraintDisplayName) \
{ \
	Builder.AddMenuEntry( \
		LOCTEXT("Create" #ConstraintType "Label", "Create " ConstraintDisplayName), \
		LOCTEXT("Create" #ConstraintType "ToolTip", "Create a " ConstraintDisplayName ".\n\nInitially setup using currently selected Rigid Body Actors, or empty."), \
		FSlateIcon(), \
		FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnCreateConstraintClicked, ConstraintType::StaticClass()), \
		NAME_None, \
		EUserInterfaceActionType::Button); \
}


void FAGX_TopMenu::FillConstraintMenu(FMenuBuilder& Builder)
{
	ADD_CREATE_CONSTRAINT_MENU_ENTRY(AAGX_CylindricalConstraint, "Cylindrical Constraint");
	ADD_CREATE_CONSTRAINT_MENU_ENTRY(AAGX_DistanceConstraint, "Distance Constraint");
	ADD_CREATE_CONSTRAINT_MENU_ENTRY(AAGX_HingeConstraint, "Hinge Constraint");
	ADD_CREATE_CONSTRAINT_MENU_ENTRY(AAGX_LockConstraint, "Lock Constraint");
	ADD_CREATE_CONSTRAINT_MENU_ENTRY(AAGX_PrismaticConstraint, "Prismatic Constraint");
}


void FAGX_TopMenu::OnCreateConstraintClicked(UClass* ConstraintClass)
{
	FAGX_EditorUtilities::CreateConstraint(
		ConstraintClass,
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);
}


void FAGX_TopMenu::OnOpenAboutDialogClicked()
{
	FText Title = LOCTEXT("AboutDialogTitle", "About AGX Unreal");

	FText Message = LOCTEXT("AboutDialogMessage",
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