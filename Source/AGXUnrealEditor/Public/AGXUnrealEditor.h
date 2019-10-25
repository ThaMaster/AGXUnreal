#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"

class FComponentVisualizer;
class FToolBarBuilder;
class FMenuBuilder;

class FAGXUnrealEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// \todo I think we can rename this to whatever we want.
	void PluginButtonClicked();

	const TSharedPtr<class FAGX_TopMenu>& GetAgxTopMenu() const;

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

	/**
	 * Registers property type customizations (IPropertyTypeCustomization),
	 * and class detail customizations (IDetailCustomization).
	 */
	void RegisterCustomizations();

	/**
	 * Unrrgisters property type customizations and class detail customizations.
	 */
	void UnregisterCustomizations();

	void RegisterComponentVisualizers();

	void UnregisterComponentVisualizers();

	void RegisterComponentVisualizer(const FName& ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);
	
	void UnregisterComponentVisualizer(const FName& ComponentClassName);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class FAGX_TopMenu> AgxTopMenu;
};

