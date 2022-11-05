// Copyright 2022, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_HeightFieldBoundsComponent.generated.h"

class ALandscape;

UCLASS(
	ClassGroup = "AGX", Category = "AGX",
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_HeightFieldBoundsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	struct FTerrainBoundsInfo
	{
		FTransform Transform;
		FVector HalfExtent;
	};

	UAGX_HeightFieldBoundsComponent();

	/**
	 * The distance from the center of the Terrain to its edges [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Height Field Bounds")
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

private:
	struct FTransformAndLandscape
	{
		FTransformAndLandscape(const ALandscape& InLandscape, const FTransform& InTransform)
			: Landscape(InLandscape)
			, Transform(InTransform)
		{
		}
		const ALandscape& Landscape;
		FTransform Transform;
	};

	/**
	 * Returns the transform of the Owner of this Component and any Landscape owned by that owner if
	 * possible. Only Terrain and HeightField owners are currently supported.
	 */
	TOptional<FTransformAndLandscape> GetLandscapeAndTransformFromOwner() const;
};
