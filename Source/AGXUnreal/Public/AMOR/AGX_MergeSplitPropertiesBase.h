// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_MergeSplitPropertiesBase.generated.h"

class UAGX_RigidBodyComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_MergeSplitPropertiesBase
{
	GENERATED_USTRUCT_BODY()

public:

	/**
	* Must be called by the owning object at begin play (after the owning object has allocated a
	* native AGX Dynamics object).
	*/
	template <typename T>
	void OnBeginPlay(T& Owner);

#if WITH_EDITOR
	/**
	 * Must be called by the owning object at PostEditChangeProperty.
	 */
	template <typename T>
	void OnPostEditChangeProperty(T& Owner);
#endif

	template <typename T>
	void CreateNative(T& Owner);

	//Todo add comment about why we doe this.
	FAGX_MergeSplitPropertiesBase& operator=(const FAGX_MergeSplitPropertiesBase& Other);

	UPROPERTY(EditAnywhere, Category = "AMOR")
	bool bEnableMerge = false;

	UPROPERTY(EditAnywhere, Category = "AMOR")
	bool bEnableSplit = false;

	void SetEnableMerge(bool bEnable);
	bool GetEnableMerge() const;

	void SetEnableSplit(bool bEnable);
	bool GetEnableSplit() const;

	bool HasNative() const;
	const FMergeSplitPropertiesBarrier& GetNative() const;
	FMergeSplitPropertiesBarrier& GetNative();

private:
	void UpdateNativeProperties();

	FMergeSplitPropertiesBarrier NativeBarrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_MergeSplitPropertiesBase in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_MergeSplitPropertiesBase_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static void SetEnableMerge(UPARAM(ref) FAGX_MergeSplitPropertiesBase& Properties, bool bEnable)
	{
		if (!Properties.HasNative())
		{
			UE_LOG(
				LogTemp, Warning, TEXT("Blueprint UFUNCTION SetEnableMerge was called on a "
					"FAGX_MergeSplitPropertiesBase without a Native AGX Dynamics object. Remember to call "
					"CreateMergeSplitProperties() on the owning object before calling this function."));
		}

		Properties.SetEnableMerge(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static bool GetEnableMerge(UPARAM(ref) const FAGX_MergeSplitPropertiesBase& Properties)
	{
		return Properties.GetEnableMerge();
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static void SetEnableSplit(UPARAM(ref) FAGX_MergeSplitPropertiesBase& Properties, bool bEnable)
	{
		if (!Properties.HasNative())
		{
			UE_LOG(
				LogTemp, Warning, TEXT("Blueprint UFUNCTION SetEnableSplit was called on a "
					"FAGX_MergeSplitPropertiesBase without a Native AGX Dynamics object. Remember to call "
					"CreateMergeSplitProperties() on the owning object before calling this function."));
		}
		Properties.SetEnableSplit(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	static bool GetEnableSplit(UPARAM(ref) const FAGX_MergeSplitPropertiesBase& Properties)
	{
		return Properties.GetEnableSplit();
	}

};
