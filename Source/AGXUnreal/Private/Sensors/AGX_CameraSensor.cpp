// Copyright 2023, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/TextureRenderTarget2D.h"

UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CameraSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsValid = CheckValid();
}

#if WITH_EDITOR
bool UAGX_CameraSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, Resolution),
			GET_MEMBER_NAME_CHECKED(ThisClass, RenderTarget)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}
#endif

bool UAGX_CameraSensorComponent::CheckValid() const
{
	if (FOV <= KINDA_SMALL_NUMBER)
	{
		const FString Msg = FString::Printf(
			TEXT(
				"Camera Sensor '%s' in Actor '%s' has an invalid FOV: %f. Please set a valid FOV."),
			*GetName(), *GetLabelSafe(GetOwner()), FOV);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (Resolution.X <= KINDA_SMALL_NUMBER || Resolution.Y <= KINDA_SMALL_NUMBER)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' has an invalid Resolution: [%s]. Please set a "
				 "valid FOV."),
			*GetName(), *GetLabelSafe(GetOwner()), *Resolution.ToString());
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (RenderTarget == nullptr)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' does not have a RenderTarget assigned to it. "
				 "Use the 'Generate Runtime Assets' button in the Details Panel to generate a "
				 "valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}
