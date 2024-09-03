// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtLambertianOpaqueMaterial;

class AGXUNREALBARRIER_API FRtLambertianOpaqueMaterialBarrier
{
public:
	FRtLambertianOpaqueMaterialBarrier();
	FRtLambertianOpaqueMaterialBarrier(std::unique_ptr<FRtLambertianOpaqueMaterial> InNative);
	FRtLambertianOpaqueMaterialBarrier(FRtLambertianOpaqueMaterialBarrier&& Other);
	virtual ~FRtLambertianOpaqueMaterialBarrier();

	void SetReflectivity(float Reflectivity);
	float GetReflectivity() const;

	virtual void AllocateNative();

	bool HasNative() const;
	FRtLambertianOpaqueMaterial* GetNative();
	const FRtLambertianOpaqueMaterial* GetNative() const;
	void ReleaseNative();

private:
	FRtLambertianOpaqueMaterialBarrier(const FRtLambertianOpaqueMaterialBarrier&) = delete;
	void operator=(const FRtLambertianOpaqueMaterialBarrier&) = delete;

protected:
	std::unique_ptr<FRtLambertianOpaqueMaterial> Native;
};
