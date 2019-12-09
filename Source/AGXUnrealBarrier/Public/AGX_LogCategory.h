// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logging/LogMacros.h"

/**
 * Declares AGX specific Unreal log category.
 *
 * Default verbosity is set to 'Log', and will be used when one is not specified in the
 * ini files or on the command line. Anything more verbose than this will not be logged.
 *
 * Compile time verbosity is set to 'All'.
 */
AGXUNREALBARRIER_API DECLARE_LOG_CATEGORY_EXTERN(LogAGX, Log, All);