// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes
#include <memory>

struct FMergeSplitPropertiesPtr;

class AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
{
public:
	FMergeSplitPropertiesBarrier();
	FMergeSplitPropertiesBarrier(std::unique_ptr<FMergeSplitPropertiesPtr> Native);
	FMergeSplitPropertiesBarrier(FMergeSplitPropertiesBarrier&& Other) noexcept;
	~FMergeSplitPropertiesBarrier();

	bool HasNative() const;
	FMergeSplitPropertiesPtr* GetNative();
	const FMergeSplitPropertiesPtr* GetNative() const;

	template <typename T>
	void AllocateNative(T& Owner);

	void SetEnableMerge(bool bEnable);
	bool GetEnableMerge() const;

	void SetEnableSplit(bool bEnable);
	bool GetEnableSplit() const;

private:
	FMergeSplitPropertiesBarrier& operator=(const FMergeSplitPropertiesBarrier& Other) = delete;

	std::unique_ptr<FMergeSplitPropertiesPtr> NativePtr;
};
