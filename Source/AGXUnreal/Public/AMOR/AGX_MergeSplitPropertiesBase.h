// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_MergeSplitPropertiesBase.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_MergeSplitPropertiesBase
{
	GENERATED_USTRUCT_BODY()

public:

	virtual ~FAGX_MergeSplitPropertiesBase() = default;

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

protected:
	FMergeSplitPropertiesBarrier NativeBarrier;
};
