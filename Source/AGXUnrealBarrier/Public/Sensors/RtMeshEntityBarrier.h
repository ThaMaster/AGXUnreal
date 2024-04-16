// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtMeshEntity;
class FRtMeshBarrier;

class AGXUNREALBARRIER_API FRtMeshEntityBarrier
{
public:
	FRtMeshEntityBarrier();
	FRtMeshEntityBarrier(std::unique_ptr<FRtMeshEntity> Native);
	FRtMeshEntityBarrier(FRtMeshEntityBarrier&& Other);
	~FRtMeshEntityBarrier();

	bool HasNative() const;
	void AllocateNative(FRtMeshBarrier& Mesh);
	FRtMeshEntity* GetNative();
	const FRtMeshEntity* GetNative() const;

	void SetTransform(const FTransform& Transform);

private:
	FRtMeshEntityBarrier(const FRtMeshEntityBarrier&) = delete;
	void operator=(const FRtMeshEntityBarrier&) = delete;

private:
	std::unique_ptr<FRtMeshEntity> NativeRef; 
};
