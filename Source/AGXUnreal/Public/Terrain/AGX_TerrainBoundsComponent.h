// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainBoundsComponent.generated.h"

UCLASS(
	ClassGroup = "AGX", Category = "AGX",
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_TerrainBoundsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	struct FTerrainBoundsInfo
	{
		FTransform Transform;
		FVector HalfExtent;
	};

	UAGX_TerrainBoundsComponent();

	/**
	 * The distance from the center of the Terrain to its edges [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	FVector HalfExtent {1000.0, 1000.0, 1000.0};

	/**
	 * Get Bounds as defined by the user, i.e. using exactly the HalfExtent UPROPERTY.
	 * Only valid if the owner of this Component is an AGX_Terrain Actor and a SourceLandscape is
	 * set in that owner.
	 */
	TOptional<FTerrainBoundsInfo> GetUserSetBounds() const;

	/**
	 * Get Bounds that are adjusted to align with the Landscape quads.
	 * Only valid if the owner of this Component is an AGX_Terrain Actor and a SourceLandscape is
	 * set in that owner.
	 */
	TOptional<FTerrainBoundsInfo> GetLandscapeAdjustedBounds() const;
};
