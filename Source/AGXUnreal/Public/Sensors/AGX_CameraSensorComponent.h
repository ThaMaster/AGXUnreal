// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

// Standard library includes.
#include <mutex>

#include "AGX_CameraSensorComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewImagePixels, const TArray<FColor>&, Image);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewImageROS2, const FAGX_SensorMsgsImage&, Image);

/**
 * EXPERIMENTAL
 *
 * Camera Sensor Component, allowing to extract camera pixel information in runtime.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CameraSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_CameraSensorComponent();

	/**
	 * Field of View (FOV) of the Camera Sensor [deg].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Camera",
		meta = (ClampMin = "0.0", ClampMax = "170.0"))
	float FOV {90.f};

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void SetFOV(float InFOV);

	/**
	 * Output resolution of the Camera Sensor [pixels].
	 * The first element is the horizontal resolution, and the second vertical resolution.
	 * Note: using a large resolution will come with a performance hit.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera", meta = (ClampMin = "1"))
	FIntPoint Resolution {256, 256};

	/**
	 * Render Target used by the Camera Sensor to write pixel data to.
	 * It is recommended to use the 'Generate Runtime Assets' button in the Details Panel to
	 * generate it.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderTarget {nullptr};

	/**
	 * Tell the Camera to capture a new image as an array of 8-bit RGB pixels. This is an
	 * asynchronous operation and is faster than the blocking GetImagePixels which synchronizes with
	 * the render thread immediately. Bind to the NewImagePixels Event to get a callback with the
	 * image data once it is ready.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void GetImagePixelsAsync();

	/**
	 * Delegate that is executed whenever a new Camera image as 8-bit RGB pixels is available.
	 * This delegate is called as the last step of a call to GetImagePixelsAsync.
	 * Users may bind to this delegate in order to get a callback.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnNewImagePixels NewImagePixels;

	/**
	 * Important: This may be a very slow operation. Use the Async version for better performance.
	 * Returns the current frame as seen by this Camera Sensor as an array of 8-bit RGB pixels.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	TArray<FColor> GetImagePixels() const;

	/**
	 * Tell the Camera to capture a new image as a ROS2 sensor_msgs::Image message. This is an
	 * asynchronous operation and is faster than the blocking GetImageROS2 which synchronizes with
	 * the render thread immediately. Bind to the NewImageROS2 Event to get a callback with the
	 * image message once it is ready.
	 * If Grayscale is set to true, only a single value (average intensity) for each pixel is
	 * set.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Camera", meta = (DisplayName = "Get Image ROS2 Async"))
	void GetImageROS2Async(bool Grayscale);

	/**
	 * Delegate that is executed whenever a new image as a ROS2 sensor_msgs::Image message is
	 * available. This delegate is called as the last step of a call to GetImageROS2Async. Users may
	 * bind to this delegate in order to get a callback.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnNewImageROS2 NewImageROS2;

	/**
	 * Important: This may be a very slow operation. Use the Async version for better performance.
	 * Returns the current frame as seen by this Camera Sensor as a ROS2 sensor_msgs::Image message.
	 * If Grayscale is set to true, only a single value (average intensity) for each pixel is
	 * set.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	FAGX_SensorMsgsImage GetImageROS2(bool Grayscale = false) const;

	static bool IsFovValid(float FOV);
	static bool IsResolutionValid(const FIntPoint& Resolution);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void PostApplyToComponent() override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject Interface

	struct FAGX_ImageBuffer
	{
		TArray<FColor> Image;
		std::mutex ImageMutex;
		bool EndPlayTriggered {false};
	};

private:
	bool bIsValid {false};
	USceneCaptureComponent2D* CaptureComponent2D;
	TSharedPtr<FAGX_ImageBuffer> LastImage;

	void Init();
	void InitCaptureComponent();
	bool CheckValid() const;
};
