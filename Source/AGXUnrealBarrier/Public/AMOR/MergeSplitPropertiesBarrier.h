// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes
#include <memory>

class FConstraintBarrier;
class FMergeSplitThresholdsBarrier;
class FRigidBodyBarrier;
class FShapeBarrier;
class FWireBarrier;

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

	template <typename T>
	static FMergeSplitPropertiesBarrier CreateFrom(T& Barrier);

	void SetEnableMerge(bool bEnable);
	bool GetEnableMerge() const;

	void SetEnableSplit(bool bEnable);
	bool GetEnableSplit() const;

	void SetShapeContactMergeSplitThresholds(FMergeSplitThresholdsBarrier* Thresholds);
	void SetConstraintMergeSplitThresholds(FMergeSplitThresholdsBarrier* Thresholds);
	void SetWireMergeSplitThresholds(FMergeSplitThresholdsBarrier* Thresholds);

	void BindToNewOwner(FRigidBodyBarrier& NewOwner);
	void BindToNewOwner(FShapeBarrier& NewOwner);
	void BindToNewOwner(FConstraintBarrier& NewOwner);
	void BindToNewOwner(FWireBarrier& NewOwner);

private:
	FMergeSplitPropertiesBarrier& operator=(const FMergeSplitPropertiesBarrier& Other) = delete;

	std::unique_ptr<FMergeSplitPropertiesPtr> NativePtr;
};
