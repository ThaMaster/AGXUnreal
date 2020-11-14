#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "NotifyBarrier.h"

class FAGXUnrealBarrierModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FNotifyBarrier NotifyBarrier;

	void SetupAgxEnvironment();
	void SetupUsePluginResourcesOnly();
};
