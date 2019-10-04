// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAGXUnrealEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/**
	 * Registers settings exposed in the Project Settings window.
	 * This is typically the default simulation settings.
	 */
	void RegisterProjectSettings();

	/**
	 * Unregisters settings exposed in the Project Settings window.
	 */
	void UnregisterProjectSettings();

	/**
	 * Registers property type customizations (IPropertyTypeCustomization),
	 * and class detail customizations (IDetailCustomization).
	 */
	void RegisterCustomizations();

	/**
	 * Unrrgisters property type customizations and class detail customizations.
	 */
	void UnregisterCustomizations();
};