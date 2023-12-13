// Copyright 2023, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"

UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAGX_CameraSensorComponent::SetFOV(float InFOV)
{
	if (InFOV < KINDA_SMALL_NUMBER || InFOV > 170.f)
		return;

	if (CaptureComponent2D != nullptr)
		CaptureComponent2D->FOVAngle = InFOV;

	FOV = InFOV;
}

FAGX_SensorMsgsImage UAGX_CameraSensorComponent::GetImageROS2(bool Grayscale) const
{
	const TArray<FColor> Pixels = GetImagePixels();
	if (Pixels.Num() == 0)
		return FAGX_SensorMsgsImage();

	const int32 SizeX = RenderTarget->SizeX;
	const int32 SizeY = RenderTarget->SizeY;
	FAGX_SensorMsgsImage Image;

	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		const float TimeStamp = Sim->GetTimeStamp();
		Image.Header.Stamp.Sec = static_cast<int32>(TimeStamp);
		float Unused;
		Image.Header.Stamp.Nanosec = static_cast<int32>(FMath::Modf(TimeStamp, &Unused) * 1.0E9f);
	}

	Image.Height = static_cast<int64>(RenderTarget->SizeY);
	Image.Width = static_cast<int64>(RenderTarget->SizeX);

	if (Grayscale)
	{
		Image.Step = SizeY * sizeof(uint8);
		Image.Encoding = TEXT("mono8");
		Image.Data.Reserve(Pixels.Num());
		for (const auto& Color : Pixels)
		{
			const uint16 Sum = static_cast<uint16>(Color.R) + static_cast<uint16>(Color.G) +
							   static_cast<uint16>(Color.B);
			Image.Data.Add(static_cast<uint8>(Sum / 3));
		}
	}
	else
	{
		Image.Step = SizeY * sizeof(uint8) * 3;
		Image.Encoding = TEXT("rgb8");
		Image.Data.Reserve(Pixels.Num() * 3);
		for (const auto& Color : Pixels)
		{
			Image.Data.Add(Color.R);
			Image.Data.Add(Color.G);
			Image.Data.Add(Color.B);
		}
	}

	return Image;
}

TArray<FColor> UAGX_CameraSensorComponent::GetImagePixels() const
{
	if (!bIsValid || RenderTarget == nullptr)
		return TArray<FColor>();

	if (RenderTarget->GetFormat() != EPixelFormat::PF_B8G8R8A8)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetImagePixels on Camera Sensor '%s' in Actor '%s' was called with a "
				 "RenderTarget of invalid format. The format should be RGBA8. Use the 'Generate "
				 "Runtime Assets' button in the Details Panel to generate a valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return TArray<FColor>();
	}

	FTextureRenderTargetResource* RtResource = RenderTarget->GameThread_GetRenderTargetResource();
	TArray<FColor> PixelData;
	RtResource->ReadPixels(PixelData);
	return PixelData;
}

void UAGX_CameraSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	Init();
}

void UAGX_CameraSensorComponent::PostApplyToComponent()
{
	Super::PostApplyToComponent();

	if (GIsReconstructingBlueprintInstances && GetWorld() && GetWorld()->IsGameWorld())
	{
		// We are in a Blueprint reconstruction during Play and we need to call Init manually to
		// ensure we are in a valid state. Otherwise CaptureComponent2D will always be nullptr since
		// it is not copied over to this new Component.
		Init();
	}
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

void UAGX_CameraSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CameraSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CameraSensorComponent, FOV),
		[](ThisClass* This) { This->SetFOV(This->FOV); });
}

#endif

void UAGX_CameraSensorComponent::Init()
{
	AGX_CHECK(CaptureComponent2D == nullptr);

	CaptureComponent2D =
		NewObject<USceneCaptureComponent2D>(this, FName(TEXT("SceneCaptureComponent2D")));
	CaptureComponent2D->RegisterComponent();
	CaptureComponent2D->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	InitCaptureComponent();
	bIsValid = CheckValid();
}

void UAGX_CameraSensorComponent::InitCaptureComponent()
{
	if (CaptureComponent2D == nullptr)
		return;

	CaptureComponent2D->TextureTarget = RenderTarget;
	CaptureComponent2D->FOVAngle = FOV;
}

bool UAGX_CameraSensorComponent::CheckValid() const
{
	if (FOV <= KINDA_SMALL_NUMBER || FOV > 170.f)
	{
		const FString Msg = FString::Printf(
			TEXT(
				"Camera Sensor '%s' in Actor '%s' has an invalid FOV: %f. Please set a valid FOV."),
			*GetName(), *GetLabelSafe(GetOwner()), FOV);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (Resolution.X <= 0 || Resolution.Y <= 0)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' has an invalid Resolution: [%s]. Please set a "
				 "valid Resolution."),
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
	else if (RenderTarget->GetFormat() != EPixelFormat::PF_B8G8R8A8)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' has a RenderTarget with an unsupported pixel "
				 "format. The pixel format should be RGBA8. Use the 'Generate Runtime Assets' "
				 "button in the Details Panel to generate a valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}
	else if (RenderTarget->SizeX != Resolution.X || RenderTarget->SizeY != Resolution.Y)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' expected to have a RenderTarget with the resolution [%s] but it was [%d %d]."
				 "Use the 'Generate Runtime Assets' button in the Details Panel to generate a valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()), *Resolution.ToString(), RenderTarget->SizeX,
			RenderTarget->SizeY);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (CaptureComponent2D == nullptr)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' does not have a CaptureComponent2D subobject."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}
