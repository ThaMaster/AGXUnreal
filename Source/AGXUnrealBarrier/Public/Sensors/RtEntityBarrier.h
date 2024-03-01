// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtEntityRef;
class FRtMeshBarrier;

class AGXUNREALBARRIER_API FRtEntityBarrier
{
public:
	FRtEntityBarrier();
	FRtEntityBarrier(std::unique_ptr<FRtEntityRef> Native);
	FRtEntityBarrier(FRtEntityBarrier&& Other);
	~FRtEntityBarrier();

	bool HasNative() const;
	void AllocateNative(FRtMeshBarrier& Mesh);
	FRtEntityRef* GetNative();
	const FRtEntityRef* GetNative() const;

	void ReleaseNative();

private:
	FRtEntityBarrier(const FRtEntityBarrier&) = delete;
	void operator=(const FRtEntityBarrier&) = delete;

private:
	std::unique_ptr<FRtEntityRef> NativeRef; 
};
