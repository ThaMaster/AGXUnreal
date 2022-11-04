// Copyright 2022, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainBoundsComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Landscape.h"

// Standard library includes.
#include <cmath>

UAGX_TerrainBoundsComponent::UAGX_TerrainBoundsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TOptional<UAGX_TerrainBoundsComponent::FTerrainBoundsInfo>
UAGX_TerrainBoundsComponent::GetUserSetBounds() const
{
	AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(GetOwner());
	if (Terrain == nullptr)
		return {};

	ALandscape* Landscape = Terrain->SourceLandscape;
	if (Landscape == nullptr)
		return {};

	if (HalfExtent.X < 0 || HalfExtent.Y < 0 || HalfExtent.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain: '%s' have bounds with negative half extent. This is not supported."),
			*Terrain->GetName());
		return {};
	}

	FTerrainBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape->GetActorQuat(), Terrain->GetActorLocation());
	BoundsInfo.HalfExtent = HalfExtent;
	return BoundsInfo;
}

/**
 * Get Bounds that are adjusted to align with the Landscape quads.
 * Only valid if the owner of this Component is an AGX_Terrain Actor and a SourceLandscape is
 * set in that owner.
 */
TOptional<UAGX_TerrainBoundsComponent::FTerrainBoundsInfo>
UAGX_TerrainBoundsComponent::GetLandscapeAdjustedBounds() const
{
	AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(GetOwner());
	if (Terrain == nullptr)
		return {};

	ALandscape* Landscape = Terrain->SourceLandscape;
	if (Landscape == nullptr)
		return {};

	if (HalfExtent.X < 0 || HalfExtent.Y < 0 || HalfExtent.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain: '%s' have bounds with negative half extent. This is not supported."),
			*Terrain->GetName());
		return {};
	}

	const FTransform BoundsWorldTrans(Landscape->GetActorQuat(), Terrain->GetActorLocation());
	const FVector Corner0World = BoundsWorldTrans.TransformPositionNoScale(
		FVector(-HalfExtent.X, -HalfExtent.Y, -HalfExtent.Z));
	const FVector Corner1World = BoundsWorldTrans.TransformPositionNoScale(
		FVector(HalfExtent.X, HalfExtent.Y, HalfExtent.Z));

	const FTransform& LandscapeTrans = Landscape->GetTransform();

	// Local here is in Landscapes coordinate system.
	const FVector Corner0Local = LandscapeTrans.InverseTransformPositionNoScale(Corner0World);
	const FVector Corner1Local = LandscapeTrans.InverseTransformPositionNoScale(Corner1World);

	const auto QuadSideSizeX = Landscape->GetActorScale().X;
	const auto QuadSideSizeY = Landscape->GetActorScale().Y;

	// "Snap" to quad grid.
	FVector Corner0LocalAdjusted(
		std::ceil(Corner0Local.X / QuadSideSizeX) * QuadSideSizeX,
		std::ceil(Corner0Local.Y / QuadSideSizeY) * QuadSideSizeY, Corner0Local.Z);
	FVector Corner1LocalAdjusted(
		std::floor(Corner1Local.X / QuadSideSizeX) * QuadSideSizeX,
		std::floor(Corner1Local.Y / QuadSideSizeY) * QuadSideSizeY, Corner1Local.Z);

	if (!FAGX_LandscapeSizeInfo::IsOpenWorldLandscape(*Landscape))
	{
		// Currently, we have found no way to detect the overall size of an open world Landscape,
		// and we will not support moving the TerrainBounds outside the edge of one.
		// For non-open world Landscapes, we do handled this, which is done here.
		auto EnsureInBounds = [](FVector& P, auto Xmin, auto Xmax, auto Ymin, auto Ymax)
		{
			if (P.X < Xmin)
				P.X = Xmin;
			else if (P.X > Xmax)
				P.X = Xmax;
			if (P.Y < Ymin)
				P.Y = Ymin;
			else if (P.Y > Ymax)
				P.Y = Ymax;
		};

		// Clamp so that we are never outside the Landscape.
		const std::tuple<float, float> SideLengths =
			FAGX_LandscapeSizeInfo::GetSideLengths(*Landscape);
		EnsureInBounds(
			Corner0LocalAdjusted, 0, std::get<0>(SideLengths), 0, std::get<1>(SideLengths));
		EnsureInBounds(
			Corner1LocalAdjusted, 0, std::get<0>(SideLengths), 0, std::get<1>(SideLengths));
	}
	const FVector Corner0AdjustedGlobal =
		LandscapeTrans.TransformPositionNoScale(Corner0LocalAdjusted);
	const FVector Corner1AdjustedGlobal =
		LandscapeTrans.TransformPositionNoScale(Corner1LocalAdjusted);

	const FVector CenterPointGlobal = (Corner0AdjustedGlobal + Corner1AdjustedGlobal) * 0.5;
	const auto HalfExtentX = (Corner1LocalAdjusted.X - Corner0LocalAdjusted.X) / 2.0;
	const auto HalfExtentY = (Corner1LocalAdjusted.Y - Corner0LocalAdjusted.Y) / 2.0;

	FTerrainBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape->GetActorQuat(), CenterPointGlobal);
	BoundsInfo.HalfExtent = FVector(HalfExtentX, HalfExtentY, HalfExtent.Z);
	return BoundsInfo;
}