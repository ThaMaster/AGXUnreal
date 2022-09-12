// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_MergeSplitPropertiesBase.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholds.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ConstraintMergeSplitProperties.generated.h"

class UAGX_ConstraintComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ConstraintMergeSplitProperties : public FAGX_MergeSplitPropertiesBase
{
	GENERATED_USTRUCT_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "AMOR")
	UAGX_ConstraintMergeSplitThresholds* Thresholds;

	/**
	* Must be called by the owning object at begin play (after the owning object has allocated a
	* native AGX Dynamics object).
	*/
	void OnBeginPlay(UAGX_ConstraintComponent& Owner);

#if WITH_EDITOR
	/**
	 * Must be called by the owning object at PostEditChangeProperty.
	 */
	void OnPostEditChangeProperty(UAGX_ConstraintComponent& Owner);
#endif

	void CreateNative(UAGX_ConstraintComponent& Owner);

	//Todo add comment about why we doe this.
	FAGX_ConstraintMergeSplitProperties& operator=(const FAGX_ConstraintMergeSplitProperties& Other);

private:
	void UpdateNativeProperties(UAGX_ConstraintComponent& Owner);
	void UpdateNativeThresholds(UAGX_ConstraintComponent& Owner);
};

/**
 * This class acts as an API that exposes functions of FAGX_ConstraintMergeSplitProperties in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintMergeSplitProperties_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static void SetEnableMerge(UPARAM(ref) FAGX_ConstraintMergeSplitProperties& Properties, bool bEnable)
	{
		if (!Properties.HasNative())
		{
			UE_LOG(
				LogTemp, Warning, TEXT("Blueprint UFUNCTION SetEnableMerge was called on a "
					"FAGX_ConstraintMergeSplitProperties without a Native AGX Dynamics object. Remember to call "
					"CreateMergeSplitProperties() on the owning object before calling this function."));
		}

		Properties.SetEnableMerge(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static bool GetEnableMerge(UPARAM(ref) const FAGX_ConstraintMergeSplitProperties& Properties)
	{
		return Properties.GetEnableMerge();
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static void SetEnableSplit(UPARAM(ref) FAGX_ConstraintMergeSplitProperties& Properties, bool bEnable)
	{
		if (!Properties.HasNative())
		{
			UE_LOG(
				LogTemp, Warning, TEXT("Blueprint UFUNCTION SetEnableSplit was called on a "
					"FAGX_ConstraintMergeSplitProperties without a Native AGX Dynamics object. Remember to call "
					"CreateMergeSplitProperties() on the owning object before calling this function."));
		}
		Properties.SetEnableSplit(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static bool GetEnableSplit(UPARAM(ref) const FAGX_ConstraintMergeSplitProperties& Properties)
	{
		return Properties.GetEnableSplit();
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static UAGX_ConstraintMergeSplitThresholds* GetThresholds(
		UPARAM(ref) const FAGX_ConstraintMergeSplitProperties& Properties)
	{
		return Properties.Thresholds;
	}
};
