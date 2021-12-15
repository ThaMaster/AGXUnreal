// Copyright 2021, Algoryx Simulation AB.


#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

/**
 * Provides helper functions for working with notifications, both when in Editor and built
 * executable.
 */
class AGXUNREAL_API FAGX_NotificationUtilities
{
public:
	/**
	 * Displays a dialog box with an OK button and adds a log message with the text. If a title is
	 * not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithLogLog(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithLogLog but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithLogLogInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Displays a dialog box with an OK button and adds a warning log message with the text. If a
	 * title is not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithWarningLog(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithWarningLog but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithWarningLogInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Displays a dialog box with an OK button and add an error log message with the text. If a
	 * title is not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithErrorLog(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithErrorLog but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithErrorLogInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");
};
