#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAGXUnrealModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/**
	 * Registers property type customizations (IPropertyTypeCustomization),
	 * and class detail customizations (IDetailCustomization).
	 */
	void RegisterCustomizations();

	void UnregisterCustomizations();
};
