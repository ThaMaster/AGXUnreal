// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_NotificationUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"

namespace
{
	void ShowDialogBox(const FString& Text, const FString& InTitle)
	{
		const FText Title = InTitle.IsEmpty() ? FText::FromString("AGX Dynamics for Unreal")
											  : FText::FromString(InTitle);
		FMessageDialog::Open(
			EAppMsgType::Ok, FText::FromString(Text),
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			&Title
#else
			Title
#endif
		);
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

bool FAGX_NotificationUtilities::YesNoQuestion(const FText& Question)
{
	const FText Title = FText::FromString("AGX Dynamics for Unreal");
	return FMessageDialog::Open(
			   EAppMsgType::YesNo, Question,
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			   &Title
#else
			   Title
#endif
			   ) == EAppReturnType::Yes;
}

void FAGX_NotificationUtilities::ShowNotification(
	const FString& Text, SNotificationItem::ECompletionState State, float Duration)
{
#if UE_VERSION_OLDER_THAN(5, 1, 0) && !WITH_EDITOR
	// We cannot show the sliding notification widget in cooked builds for UE versions older
	// than 5.1 because it requires stuff from the EditorStyle module in those versions which we
	// cannot use cooked builds. So for that case we just show a regular message box.
	if (State == SNotificationItem::ECompletionState::CS_Fail)
		ShowDialogBoxWithErrorLog(Text);
	else
		ShowDialogBoxWithLogLog(Text);

#else
	FNotificationInfo Info(FText::FromString(Text));

	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = Duration;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;

	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(State);
	NotificationItem->ExpireAndFadeout();

	if (State == SNotificationItem::ECompletionState::CS_Fail)
	{
		UE_LOG(LogAGX, Error, TEXT("%s"), *Text);
	}
	else
	{
		UE_LOG(LogAGX, Log, TEXT("%s"), *Text);
	}
#endif
}
