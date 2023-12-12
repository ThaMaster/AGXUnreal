// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

#include "AGX_CameraSensorComponent.generated.h"

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
		meta = (ClampMin = "0.0", ClampMax = "120.0"))
	float FOV {90.f};

	UFUNCTION(BlueprintCallable, Category = "AGX Shovel")
	void SetFOV(float InFOV);

	/**
	 * Output resolution of the Camera Sensor [pixels].
	 * The first element is the horizontal resolution, and the second vertical resolution.
	 * Note: using a large resolution will come with a performance hit.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera", meta = (ClampMin = "0.0"))
	FVector2D Resolution {256.0, 256.0};

	/**
	 * Render Target used by the Camera Sensor to write pixel data to.
	 * It is recommended to use the 'Generate Runtime Assets' button in the Details Panel to
	 * generate it.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderTarget {nullptr};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void PostApplyToComponent() override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject Interface

private:
	bool bIsValid {false};
	USceneCaptureComponent2D* CaptureComponent2D;

	void Init();
	void InitCaptureComponent();
	bool CheckValid() const;
};
