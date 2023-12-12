// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

class UTextureRenderTarget2D;

#include "AGX_CameraSensorComponent.generated.h"

/**
 * EXPERIMENTAL
 *
 * Camera Sensor Component, allowing to extract camera pixel information in runtime.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Experimental, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CameraSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_CameraSensorComponent();

	/**
	 * Field of View (FOV) of the Camera Sensor [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera", meta = (ClampMin = "0.0", ClampMax = "120.0"))
	double FOV {90.0};

	/**
	 * Output resolution of the Camera Sensor [pixels].
	 * The first element is the horizontal resolution, and the second vertical resolution.
	 * Note: using a large resolution will come with a performance hit.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Camera", meta = (ClampMin = "0.0"))
	FVector2D Resolution {256, 256};


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Camera")
	UTextureRenderTarget2D* RenderTarget {nullptr};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UObject Interface
	

private:
	bool bIsValid {false};

	bool CheckValid() const;
};
