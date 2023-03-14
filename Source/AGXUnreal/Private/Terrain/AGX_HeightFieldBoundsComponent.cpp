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

	int32 DistanceToClosestVertex(double Distance, double QuadSize)
	{
		return FMath::RoundToInt32(FMath::RoundToDouble(Distance / QuadSize));
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
	const int32 QuadSizeX = Landscape.GetActorScale().X;
	const int32 QuadSizeY = Landscape.GetActorScale().Y;

	const FVector SelectedHalfExtent =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtent.X < 0 || SelectedHalfExtent.Y < 0 || SelectedHalfExtent.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with negative half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	const FTransform& OwnerTransform = TransformAndLandscape->Transform;
	const FVector OwnerPosLocal =
		Landscape.GetActorTransform().InverseTransformPositionNoScale(OwnerTransform.GetLocation());
	int32 VertexCountX;
	int32 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);
	const int32 ClosestVertexX = DistanceToClosestVertex(OwnerPosLocal.X, QuadSizeX);
	const int32 ClosestVertexY = DistanceToClosestVertex(OwnerPosLocal.Y, QuadSizeY);

	int32 HalfExtentVertsX = DistanceToClosestVertex(SelectedHalfExtent.X, QuadSizeX);
	int32 HalfExtentVertsY = DistanceToClosestVertex(SelectedHalfExtent.Y, QuadSizeY);

	const FVector BoundPosGlobal = Landscape.GetActorTransform().TransformPositionNoScale(
		FVector(ClosestVertexX * QuadSizeX, ClosestVertexY * QuadSizeY, 0));

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent =
		FVector(HalfExtentVertsX * QuadSizeX, HalfExtentVertsY * QuadSizeY, SelectedHalfExtent.Z);

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
	const int32 QuadSizeX = Landscape.GetActorScale().X;
	const int32 QuadSizeY = Landscape.GetActorScale().Y;

	const FVector SelectedHalfExtent =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtent.X < 0 || SelectedHalfExtent.Y < 0 || SelectedHalfExtent.Z < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with negative half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	const FTransform& OwnerTransform = TransformAndLandscape->Transform;
	const FVector OwnerPosLocal = Landscape.GetActorTransform().InverseTransformPositionNoScale(
		OwnerTransform.GetLocation());
	int32 VertexCountX;
	int32 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);
	const int32 ClosestVertexX = DistanceToClosestVertex(OwnerPosLocal.X, QuadSizeX);
	const int32 ClosestVertexY = DistanceToClosestVertex(OwnerPosLocal.Y, QuadSizeY);

	if (ClosestVertexX <= 0 || ClosestVertexX > VertexCountX || ClosestVertexY <= 0 ||
		ClosestVertexY > VertexCountY)
		return {};

	int32 HalfExtentVertsX = DistanceToClosestVertex(SelectedHalfExtent.X, QuadSizeX);
	int32 HalfExtentVertsY = DistanceToClosestVertex(SelectedHalfExtent.Y, QuadSizeY);

	// Ensure we are not outside the Landscape edge.
	HalfExtentVertsX = std::min(HalfExtentVertsX, VertexCountX - ClosestVertexX);
	HalfExtentVertsX = std::min(HalfExtentVertsX, ClosestVertexX - 0);
	HalfExtentVertsY = std::min(HalfExtentVertsY, VertexCountY - ClosestVertexY);
	HalfExtentVertsY = std::min(HalfExtentVertsY, ClosestVertexY - 0);

	if (HalfExtentVertsX == 0 || HalfExtentVertsY == 0)
		return {};

	const FVector BoundPosGlobal = Landscape.GetActorTransform().TransformPositionNoScale(
		FVector(ClosestVertexX * QuadSizeX, ClosestVertexY * QuadSizeY, 0));

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent =
		FVector(HalfExtentVertsX * QuadSizeX, HalfExtentVertsY * QuadSizeY, SelectedHalfExtent.Z);

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
