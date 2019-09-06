// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Declares AGX specific Unreal log category.
 *
 * Default verbosity is set to 'Log', and will be used when one is not specified in the
 * ini files or on the command line. Anything more verbose than this will not be logged.
 * 
 * Compile time verbosity is set to 'All'.
 */
DECLARE_LOG_CATEGORY_EXTERN(LogAGX, Log, All);


class FAGXUnrealModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
