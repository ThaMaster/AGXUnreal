// Copyright 2023, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ROS2Utilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TextureResource.h"


UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CameraSensorComponent::SetFOV(float InFOV)
{
	if (!IsFovValid(InFOV))
		return;

	if (CaptureComponent2D != nullptr)
		CaptureComponent2D->FOVAngle = InFOV;

	FOV = InFOV;
}

namespace AGX_CameraSensorComponent_helpers
{
	void GetImageAsync(
		UTextureRenderTarget2D* RenderTarget,
		TSharedPtr<UAGX_CameraSensorComponent::FAGX_ImageBuffer> OutImg,
		FOnNewImagePixels& ImagePixelDelegate, FOnNewImageROS2& ImageROS2Delegate,
		const FIntPoint& Resolution, float TimeStamp, bool AsROS2Msg, bool Grayscale)
	{
		if (RenderTarget == nullptr || OutImg == nullptr)
			return;

		const FIntRect Rectangle(0, 0, RenderTarget->SizeX, RenderTarget->SizeY);

		// Read the render target surface data back.
		struct FReadSurfaceContext
		{
			FRenderTarget* SrcRenderTarget;
			TArray<FColor>* OutData;
			FIntRect Rect;
			FReadSurfaceDataFlags Flags;
		};

		auto Rt = RenderTarget->GameThread_GetRenderTargetResource();
		FReadSurfaceContext Context = {
			Rt, &OutImg->Image, Rectangle, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)};

		ENQUEUE_RENDER_COMMAND(FAGX_ReadRtCommand)
		(
			[Context, OutImg, &ImagePixelDelegate, &ImageROS2Delegate, Resolution, AsROS2Msg,
			 TimeStamp, Grayscale](FRHICommandListImmediate& RHICmdList)
			{
				{
					std::scoped_lock<std::mutex> sl(OutImg->ImageMutex);
					RHICmdList.ReadSurfaceData(
						Context.SrcRenderTarget->GetRenderTargetTexture(), Context.Rect,
						*Context.OutData, Context.Flags);
				}

				// clang-format off
				FFunctionGraphTask::CreateAndDispatchWhenReady(
					[OutImg, &ImagePixelDelegate, &ImageROS2Delegate, Resolution, AsROS2Msg, TimeStamp, Grayscale]()
					{
						std::lock_guard<std::mutex> lg(OutImg->ImageMutex);
						if (OutImg->EndPlayTriggered)
							return;
						if (AsROS2Msg)
							ImageROS2Delegate.Broadcast(FAGX_ROS2Utilities::Convert(
								OutImg->Image, TimeStamp, Resolution, Grayscale));
						else
							ImagePixelDelegate.Broadcast(OutImg->Image);
					},
					TStatId {}, nullptr,
					ENamedThreads::GameThread);
				// clang-format on
			});
	}
}

void UAGX_CameraSensorComponent::GetImagePixelsAsync()
{
	if (!bIsValid || RenderTarget == nullptr)
		return;

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	AGX_CameraSensorComponent_helpers::GetImageAsync(
		RenderTarget, LastImage, NewImagePixels, NewImageROS2, Resolution, TimeStamp, false, false);
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

void UAGX_CameraSensorComponent::GetImageROS2Async(bool Grayscale)
{
	if (!bIsValid || RenderTarget == nullptr)
		return;

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	AGX_CameraSensorComponent_helpers::GetImageAsync(
		RenderTarget, LastImage, NewImagePixels, NewImageROS2, Resolution, TimeStamp, true,
		Grayscale);
}

FAGX_SensorMsgsImage UAGX_CameraSensorComponent::GetImageROS2(bool Grayscale) const
{
	const TArray<FColor> Pixels = GetImagePixels();
	if (Pixels.Num() == 0)
		return FAGX_SensorMsgsImage();

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	return FAGX_ROS2Utilities::Convert(Pixels, TimeStamp, Resolution, Grayscale);
}

bool UAGX_CameraSensorComponent::IsFovValid(float FOV)
{
	return FOV > KINDA_SMALL_NUMBER && FOV <= 170.f;
}

bool UAGX_CameraSensorComponent::IsResolutionValid(const FIntPoint& Resolution)
{
	return Resolution.X >= 1 && Resolution.Y >= 1;
}

void UAGX_CameraSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	Init();
}

void UAGX_CameraSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	NewImagePixels.Clear();

	if (LastImage != nullptr)
		LastImage->EndPlayTriggered = true;
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

	if (LastImage == nullptr)
	{
		LastImage = MakeShared<FAGX_ImageBuffer>();
	}

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
	if (!IsFovValid(FOV))
	{
		const FString Msg = FString::Printf(
			TEXT(
				"Camera Sensor '%s' in Actor '%s' has an invalid FOV: %f. Please set a valid FOV."),
			*GetName(), *GetLabelSafe(GetOwner()), FOV);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (!IsResolutionValid(Resolution))
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
			TEXT("Camera Sensor '%s' in Actor '%s' expected to have a RenderTarget with the "
				 "resolution [%s] but it was [%d %d]."
				 "Use the 'Generate Runtime Assets' button in the Details Panel to generate a "
				 "valid RenderTarget."),
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
