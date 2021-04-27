#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "NotifyBarrier.h"

class FAGXNotifyListener;

class FAGXUnrealBarrierModule : public IModuleInterface
{
public:
	// ~Begin IModuleInterface interface.
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// ~End IModuleInterface interface.

	static FAGXUnrealBarrierModule& Get();

	void AddNotifyListener(FAGXNotifyListener* Listener);
	void RemoveNotifyListener(FAGXNotifyListener* Listener);
	void RelayNotifyMessage(const FString& Message, ELogVerbosity::Type Verbosity);

private:
	void SetupAgxEnvironment();
	void SetupUsePluginResourcesOnly();

private:
	FNotifyBarrier NotifyBarrier;
	void* VdbGridLibHandle;

	TArray<FAGXNotifyListener*> NotifyListeners;
};

class AGXUNREALBARRIER_API FAGXNotifyListener
{
public:
	FAGXNotifyListener();
	~FAGXNotifyListener();

	virtual void OnMessage(const FString& Message, ELogVerbosity::Type Verbosity) = 0;
};
