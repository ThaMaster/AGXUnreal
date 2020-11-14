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
#if defined(_WIN64)
	void* VdbGridLibHandle;
#endif

	void SetupAgxEnvironment();
	void SetupUsePluginResourcesOnly();
};
