// Copyright 2023, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ROS2Utilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "RenderingThread.h"
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
	struct FAGX_ImageAsyncParams
	{
		FAGX_ImageAsyncParams(
			const FIntPoint& InResolution, float InTimeStamp, bool InAsROS2Msg, bool InGrayscale)
			: Resolution(InResolution)
			, TimeStamp(InTimeStamp)
			, bAsROS2Msg(InAsROS2Msg)
			, bGrayscale(InGrayscale)
		{
		}

		FIntPoint Resolution;
		float TimeStamp {0.f};
		bool bAsROS2Msg {false};
		bool bGrayscale {false};
	};

	void GetImageAsync(
		UTextureRenderTarget2D* RenderTarget,
		TSharedPtr<UAGX_CameraSensorComponent::FAGX_ImageBuffer> OutImg, int32 ImageIndex,
		FOnNewImagePixels& ImagePixelDelegate, FOnNewImageROS2& ImageROS2Delegate,
		const FAGX_ImageAsyncParams& Params)
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
			Rt, &OutImg->Image[ImageIndex], Rectangle,
			FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)};

		ENQUEUE_RENDER_COMMAND(FAGX_ReadRtCommand)
		(
			[Context, OutImg, ImageIndex, &ImagePixelDelegate, &ImageROS2Delegate,
			 Params](FRHICommandListImmediate& RHICmdList)
			{
				{
					std::scoped_lock<std::mutex> sl(OutImg->ImageMutex);
					RHICmdList.ReadSurfaceData(
						Context.SrcRenderTarget->GetRenderTargetTexture(), Context.Rect,
						*Context.OutData, Context.Flags);
				}

				// clang-format off
				FFunctionGraphTask::CreateAndDispatchWhenReady(
					[OutImg, ImageIndex, &ImagePixelDelegate, &ImageROS2Delegate, Params]()
					{
						std::lock_guard<std::mutex> lg(OutImg->ImageMutex);
						if (OutImg->EndPlayTriggered)
							return;
						if (Params.bAsROS2Msg)
							ImageROS2Delegate.Broadcast(FAGX_ROS2Utilities::Convert(
								OutImg->Image[ImageIndex], Params.TimeStamp, Params.Resolution, Params.bGrayscale));
						else
							ImagePixelDelegate.Broadcast(OutImg->Image[ImageIndex]);
					},
					TStatId {}, nullptr,
					ENamedThreads::GameThread);
				// clang-format on
			});
	}
}

void UAGX_CameraSensorComponent::GetImagePixelsAsync()
{
	using namespace AGX_CameraSensorComponent_helpers;

	if (!bIsValid || RenderTarget == nullptr)
		return;

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	const FAGX_ImageAsyncParams Params(Resolution, TimeStamp, false, false);
	const int32 ImageIndex = LastImage->BufferHead;
	OnAsyncImageRequest();

	AGX_CameraSensorComponent_helpers::GetImageAsync(
		RenderTarget, LastImage, ImageIndex, NewImagePixels, NewImageROS2, Params);
}

TArray<FColor> UAGX_CameraSensorComponent::GetImagePixels() const
{
	if (!bIsValid)
		return TArray<FColor>();

	return UAGX_BlueprintUtilities::GetImagePixels(RenderTarget);
}

void UAGX_CameraSensorComponent::GetImageROS2Async(bool Grayscale)
{
	using namespace AGX_CameraSensorComponent_helpers;

	if (!bIsValid || RenderTarget == nullptr)
		return;

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	const FAGX_ImageAsyncParams Params(Resolution, TimeStamp, true, Grayscale);
	const int32 ImageIndex = LastImage->BufferHead;
	OnAsyncImageRequest();
	GetImageAsync(RenderTarget, LastImage, ImageIndex, NewImagePixels, NewImageROS2, Params);
}

FAGX_SensorMsgsImage UAGX_CameraSensorComponent::GetImageROS2(bool Grayscale) const
{
	if (!bIsValid)
		return FAGX_SensorMsgsImage();

	float TimeStamp = 0.f;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		TimeStamp = Sim->GetTimeStamp();

	return UAGX_BlueprintUtilities::GetImageROS2(RenderTarget, TimeStamp, Grayscale);
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
	NewImageROS2.Clear();

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
		// The reason we do this in PostApplyToComponent is because that's what was found to be
		// called after the new Component has been created and given all of it's properties from the
		// previous Component. There might be a better place to do this, but it is currently not
		// know what that place would be.
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
				 "resolution [%s] but it was [%d %d]. "
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

void UAGX_CameraSensorComponent::OnAsyncImageRequest()
{
	AGX_CHECK(LastImage->BufferHead < 2);
	LastImage->BufferHead = (LastImage->BufferHead + 1) % 2;
}
