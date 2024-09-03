// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_SurfaceMaterialAssetUserData.generated.h"

UCLASS(BlueprintType, ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_SurfaceMaterialAssetUserData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float Reflectivity{0.7}; // Todo: pointer to a SurfaceMaterial (base)
};
