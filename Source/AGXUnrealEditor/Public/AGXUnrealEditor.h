#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"

class FToolBarBuilder;
class FMenuBuilder;

class FAGXUnrealEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// \todo I think we can rename this to whatever we want.
	void PluginButtonClicked();

private:
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	/**
	 * Registers settings exposed in the Project Settings window.
	 * This is typically the default simulation settings.
	 */
	void RegisterProjectSettings();

	/**
	 * Unregisters settings exposed in the Project Settings window.
	 */
	void UnregisterProjectSettings();

	void RegisterCommands();
	void UnregisterCommands();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
