// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_SurfaceMaterialAssetUserData.generated.h"

UCLASS(abstract)
class AGXUNREAL_API UAGX_SurfaceMaterialAssetUserData : public UObject,
														public IInterface_AssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 TestInt {1};
};
