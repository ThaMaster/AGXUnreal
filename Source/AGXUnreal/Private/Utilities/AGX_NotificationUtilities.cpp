// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_NotificationUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Misc/MessageDialog.h"

namespace
{
	void ShowDialogBox(const FString& Text, const FString& InTitle)
	{
		const FText Title = InTitle.IsEmpty() ? FText::FromString("AGX Dynamics for Unreal")
											  : FText::FromString(InTitle);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Text), &Title);
	}
}

void FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(const FString& Text, const FString& Title)
{
	UE_LOG(LogAGX, Log, TEXT("%s"), *Text);
	ShowDialogBox(Text, Title);
}

void FAGX_NotificationUtilities::ShowDialogBoxWithLogLogInEditor(
	const FString& Text, UWorld* World, const FString& Title)
{
	if (World && World->IsGameWorld())
	{
		// Write only to the log during Play.
		UE_LOG(LogAGX, Log, TEXT("%s"), *Text);
	}
	else
	{
		ShowDialogBoxWithLogLog(Text, Title);
	}
}

void FAGX_NotificationUtilities::ShowDialogBoxWithWarningLog(
	const FString& Text, const FString& Title)
{
	UE_LOG(LogAGX, Warning, TEXT("%s"), *Text);
	ShowDialogBox(Text, Title);
}

void FAGX_NotificationUtilities::ShowDialogBoxWithWarningLogInEditor(
	const FString& Text, UWorld* World, const FString& Title)
{
	if (World && World->IsGameWorld())
	{
		// Write only to the log during Play.
		UE_LOG(LogAGX, Warning, TEXT("%s"), *Text);
	}
	else
	{
		ShowDialogBoxWithWarningLog(Text, Title);
	}
}

void FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
	const FString& Text, const FString& Title)
{
	UE_LOG(LogAGX, Error, TEXT("%s"), *Text);
	ShowDialogBox(Text, Title);
}

void FAGX_NotificationUtilities::ShowDialogBoxWithErrorLogInEditor(
	const FString& Text, UWorld* World, const FString& Title)
{
	if (World && World->IsGameWorld())
	{
		// Write only to the log during Play.
		UE_LOG(LogAGX, Error, TEXT("%s"), *Text);
	}
	else
	{
		ShowDialogBoxWithErrorLog(Text, Title);
	}
}

void FAGX_NotificationUtilities::LogWarningIfAmorDisabled(const FString& OwningType)
{
	const UAGX_Simulation* Simulation = GetDefault<UAGX_Simulation>();
	if (Simulation == nullptr)
	{
		return;
	}

	if (!Simulation->bEnableAMOR)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AMOR enabled on a %s, but disabled globally. Enable AMOR in Project "
				 "Settings > Plugins > AGX Dynamics for this change to have an effect."),
			*OwningType);
	}
}