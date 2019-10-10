#pragma once

/**
 * Creates the AGX Top Menu on the Unreal Main Menu Bar.
 */
class AGXUNREALEDITOR_API FAGX_TopMenu
{

public:

	FAGX_TopMenu();
	virtual ~FAGX_TopMenu();

private:

	static void CreateTopMenu(FMenuBarBuilder& Builder);
	virtual void FillTopMenu(FMenuBuilder& Builder); // Must be virtual because of dirty hack (see comment in CreateTopMenu)!
	void FillConstraintMenu(FMenuBuilder& Builder);

	void OnCreateConstraintClicked(UClass* ConstraintClass);
	void OnOpenAboutDialogClicked();

	TSharedPtr<class FExtender> Extender;
	TSharedPtr<const class FExtensionBase> UnrealMenuBarExtension;
};