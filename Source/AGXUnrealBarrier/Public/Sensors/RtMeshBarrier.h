// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtMeshRef;

class AGXUNREALBARRIER_API FRtMeshBarrier
{
public:
	FRtMeshBarrier();
	FRtMeshBarrier(std::unique_ptr<FRtMeshRef> Native);
	FRtMeshBarrier(FRtMeshBarrier&& Other);
	~FRtMeshBarrier();

	bool HasNative() const;
	void AllocateNative(const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices);
	FRtMeshRef* GetNative();
	const FRtMeshRef* GetNative() const;

	void ReleaseNative();

private:
	FRtMeshBarrier(const FRtMeshBarrier&) = delete;
	void operator=(const FRtMeshBarrier&) = delete;

private:
	std::unique_ptr<FRtMeshRef> NativeRef;
};
