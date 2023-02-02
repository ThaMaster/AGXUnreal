// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "Shapes/AGX_HeightFieldShapeComponent.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Landscape.h"

// Standard library includes.
#include <cmath>

UAGX_HeightFieldBoundsComponent::UAGX_HeightFieldBoundsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace AGX_HeightFieldBoundsComponent_helpers
{
	FVector GetInfinateOrUserSelectedBounds(
		bool Infinite, const ALandscape& Landscape, const FVector& UserSelected)
	{
		if (!Infinite)
		{
			return UserSelected;
		}

		// Use the Landscape size.
		// At scale = 1, the height span is +- 256 cm
		// https://docs.unrealengine.com/en-US/Engine/Landscape/TechnicalGuide/#calculatingheightmapzscale
		const double HeightSpanHalf = 256.0 * Landscape.GetActorScale3D().Z;

		// Here, we take a "shortcut" of using an arbitrary large value. Calculating a bounding box
		// given the Landscape transform and this Component's owning transform along with Landscape
		// size information, taking into account the rotation of the landscape etc could be done,
		// but is is unnecessarily complicated. Really, we just want a really large bound that will
		// include everything.
		static constexpr double LargeNumber = 1.e+10f;
		return FVector(LargeNumber, LargeNumber, HeightSpanHalf);
	}
}

TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo>
UAGX_HeightFieldBoundsComponent::GetUserSetBounds() const
{
	using namespace AGX_HeightFieldBoundsComponent_helpers;
	TOptional<FTransformAndLandscape> TransformAndLandscape = GetLandscapeAndTransformFromOwner();
	if (!TransformAndLandscape.IsSet())
	{
		return {};
	}

	const ALandscape& Landscape = TransformAndLandscape->Landscape;
	const FTransform& OwnerTransform = TransformAndLandscape->Transform;

	const FVector SelectedHalfExtentBounds =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtentBounds.X < 0 || SelectedHalfExtentBounds.Y < 0 ||
		SelectedHalfExtentBounds.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with negative half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorQuat(), OwnerTransform.GetLocation());
	BoundsInfo.HalfExtent = SelectedHalfExtentBounds;
	return BoundsInfo;
}

TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo>
UAGX_HeightFieldBoundsComponent::GetLandscapeAdjustedBounds() const
{
	using namespace AGX_HeightFieldBoundsComponent_helpers;
	TOptional<FTransformAndLandscape> TransformAndLandscape = GetLandscapeAndTransformFromOwner();
	if (!TransformAndLandscape.IsSet())
	{
		return {};
	}

	const ALandscape& Landscape = TransformAndLandscape->Landscape;

	const FVector SelectedHalfExtentBounds =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtentBounds.X < 0 || SelectedHalfExtentBounds.Y < 0 ||
		SelectedHalfExtentBounds.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with negative half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	const FTransform& OwnerTransform = TransformAndLandscape->Transform;

	const FTransform BoundsWorldTrans(Landscape.GetActorQuat(), OwnerTransform.GetLocation());
	const FVector Corner0World =
		BoundsWorldTrans.TransformPositionNoScale(-SelectedHalfExtentBounds);
	const FVector Corner1World =
		BoundsWorldTrans.TransformPositionNoScale(SelectedHalfExtentBounds);

	const FTransform& LandscapeTrans = Landscape.GetTransform();

	// Local here is in Landscapes coordinate system.
	const FVector Corner0Local = LandscapeTrans.InverseTransformPositionNoScale(Corner0World);
	const FVector Corner1Local = LandscapeTrans.InverseTransformPositionNoScale(Corner1World);

	const auto QuadSideSizeX = Landscape.GetActorScale().X;
	const auto QuadSideSizeY = Landscape.GetActorScale().Y;

	// "Snap" to quad grid.
	FVector Corner0LocalAdjusted(
		std::ceil(Corner0Local.X / QuadSideSizeX) * QuadSideSizeX,
		std::ceil(Corner0Local.Y / QuadSideSizeY) * QuadSideSizeY, Corner0Local.Z);
	FVector Corner1LocalAdjusted(
		std::floor(Corner1Local.X / QuadSideSizeX) * QuadSideSizeX,
		std::floor(Corner1Local.Y / QuadSideSizeY) * QuadSideSizeY, Corner1Local.Z);

	// Clamp so that we are never outside the Landscape.
	const std::tuple<double, double> SideLengths =
		AGX_HeightFieldUtilities::GetLandscapeSizeXY(Landscape);
	Corner0LocalAdjusted.X =
		FMath::Clamp<double>(Corner0LocalAdjusted.X, 0.0, std::get<0>(SideLengths));
	Corner0LocalAdjusted.Y =
		FMath::Clamp<double>(Corner0LocalAdjusted.Y, 0.0, std::get<1>(SideLengths));
	Corner1LocalAdjusted.X =
		FMath::Clamp<double>(Corner1LocalAdjusted.X, 0.0, std::get<0>(SideLengths));
	Corner1LocalAdjusted.Y =
		FMath::Clamp<double>(Corner1LocalAdjusted.Y, 0.0, std::get<1>(SideLengths));

	const FVector Corner0AdjustedGlobal =
		LandscapeTrans.TransformPositionNoScale(Corner0LocalAdjusted);
	const FVector Corner1AdjustedGlobal =
		LandscapeTrans.TransformPositionNoScale(Corner1LocalAdjusted);

	const FVector CenterPointGlobal = (Corner0AdjustedGlobal + Corner1AdjustedGlobal) * 0.5;
	const auto HalfExtentX = (Corner1LocalAdjusted.X - Corner0LocalAdjusted.X) / 2.0;
	const auto HalfExtentY = (Corner1LocalAdjusted.Y - Corner0LocalAdjusted.Y) / 2.0;

	if (HalfExtentX <= 0.0 || HalfExtentY <= 0.0)
	{
		return {};
	}

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorQuat(), CenterPointGlobal);
	BoundsInfo.HalfExtent = FVector(HalfExtentX, HalfExtentY, SelectedHalfExtentBounds.Z);
	return BoundsInfo;
}

TOptional<UAGX_HeightFieldBoundsComponent::FTransformAndLandscape>
UAGX_HeightFieldBoundsComponent::GetLandscapeAndTransformFromOwner() const
{
	if (AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(GetOwner()))
	{
		if (Terrain->SourceLandscape != nullptr)
		{
			return FTransformAndLandscape(*Terrain->SourceLandscape, Terrain->GetActorTransform());
		}
	}

	if (UAGX_HeightFieldShapeComponent* HeightField =
			Cast<UAGX_HeightFieldShapeComponent>(GetOuter()))
	{
		if (HeightField->SourceLandscape != nullptr)
		{
			return FTransformAndLandscape(
				*HeightField->SourceLandscape, HeightField->GetComponentTransform());
		}
	}

	return {};
}
