// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtSurfaceMaterial;

class AGXUNREALBARRIER_API FRtSurfaceMaterialBarrier
{
public:
	FRtSurfaceMaterialBarrier();
	FRtSurfaceMaterialBarrier(std::unique_ptr<FRtSurfaceMaterial> InNative);
	FRtSurfaceMaterialBarrier(FRtSurfaceMaterialBarrier&& Other);
	virtual ~FRtSurfaceMaterialBarrier();

	void SetReflectivity(float Reflectivity);
	float GetReflectivity() const;

	virtual void AllocateNative();

	bool HasNative() const;
	FRtSurfaceMaterial* GetNative();
	const FRtSurfaceMaterial* GetNative() const;
	void ReleaseNative();

private:
	FRtSurfaceMaterialBarrier(const FRtSurfaceMaterialBarrier&) = delete;
	void operator=(const FRtSurfaceMaterialBarrier&) = delete;

protected:
	std::unique_ptr<FRtSurfaceMaterial> Native;
};
