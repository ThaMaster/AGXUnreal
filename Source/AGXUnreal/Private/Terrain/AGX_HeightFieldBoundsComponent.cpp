// Copyright 2023, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Shapes/AGX_HeightFieldShapeComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_HeightFieldUtilities.h"

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

		// Here, we take a "shortcut" of using an arbitrary large value. Calculating a bounding box
		// given the Landscape transform and this Component's owning transform along with Landscape
		// size information, taking into account the rotation of the landscape etc could be done,
		// but is is unnecessarily complicated. Really, we just want a really large bound that will
		// include everything.
		static constexpr double LargeNumber = 1.e+10f;

		// The z-value has no effect on the simulation at all, and is purely visual. We set a rather
		// low value for it because it is easier to see where the bounds actually are in that case.
		static constexpr double HalfExtentZ = 100.f;
		return FVector(LargeNumber, LargeNumber, HalfExtentZ);
	}

	int32 GetClosestVertexIndex(double Distance, double QuadSize)
	{
		return FMath::RoundToInt(FMath::RoundToDouble(Distance / QuadSize));
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
	const double QuadSizeX = Landscape.GetActorScale().X;
	const double QuadSizeY = Landscape.GetActorScale().Y;

	const FVector SelectedHalfExtent =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtent.X <= 0.0 || SelectedHalfExtent.Y <= 0.0 || SelectedHalfExtent.Z <= 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with non-positive half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	int32 VertexCountX;
	int32 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);

	const FVector CenterPosLocal = [&]()
	{
		if (bInfiniteBounds)
		{
			return FVector(
				QuadSizeX * static_cast<double>(VertexCountX) / 2.0,
				QuadSizeY * static_cast<double>(VertexCountY) / 2.0, 0.0);
		}
		else
		{
			return Landscape.GetActorTransform().InverseTransformPositionNoScale(
				TransformAndLandscape->Transform.GetLocation());
		}
	}();

	const int32 ClosestVertexX = GetClosestVertexIndex(CenterPosLocal.X, QuadSizeX);
	const int32 ClosestVertexY = GetClosestVertexIndex(CenterPosLocal.Y, QuadSizeY);

	int32 HalfExtentVertsX = GetClosestVertexIndex(SelectedHalfExtent.X, QuadSizeX);
	int32 HalfExtentVertsY = GetClosestVertexIndex(SelectedHalfExtent.Y, QuadSizeY);

	const FVector BoundPosGlobal = Landscape.GetActorTransform().TransformPositionNoScale(FVector(
		static_cast<double>(ClosestVertexX) * QuadSizeX,
		static_cast<double>(ClosestVertexY) * QuadSizeY, 0.0));

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent = FVector(
		static_cast<double>(HalfExtentVertsX) * QuadSizeX,
		static_cast<double>(HalfExtentVertsY) * QuadSizeY, SelectedHalfExtent.Z);

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

	if (SelectedHalfExtent.X <= 0.0 || SelectedHalfExtent.Y <= 0.0 || SelectedHalfExtent.Z <= 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with non-positive half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	int32 VertexCountX;
	int32 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);

	const FVector CenterPosLocal = [&]()
	{
		if (bInfiniteBounds)
		{
			return FVector(
				QuadSizeX * static_cast<double>(VertexCountX) / 2.0,
				QuadSizeY * static_cast<double>(VertexCountY) / 2.0, 0.0);
		}
		else
		{
			return Landscape.GetActorTransform().InverseTransformPositionNoScale(
				TransformAndLandscape->Transform.GetLocation());
		}
	}();

	const int32 ClosestVertexX = GetClosestVertexIndex(CenterPosLocal.X, QuadSizeX);
	const int32 ClosestVertexY = GetClosestVertexIndex(CenterPosLocal.Y, QuadSizeY);

	if (ClosestVertexX <= 0 || ClosestVertexX > VertexCountX || ClosestVertexY <= 0 ||
		ClosestVertexY > VertexCountY)
		return {};

	int32 HalfExtentVertsX = GetClosestVertexIndex(SelectedHalfExtent.X, QuadSizeX);
	int32 HalfExtentVertsY = GetClosestVertexIndex(SelectedHalfExtent.Y, QuadSizeY);

	// Ensure we are not outside the Landscape edge.
	HalfExtentVertsX = std::min(HalfExtentVertsX, VertexCountX - ClosestVertexX - 1);
	HalfExtentVertsX = std::min(HalfExtentVertsX, ClosestVertexX - 0);
	HalfExtentVertsY = std::min(HalfExtentVertsY, VertexCountY - ClosestVertexY - 1);
	HalfExtentVertsY = std::min(HalfExtentVertsY, ClosestVertexY - 0);

	if (HalfExtentVertsX == 0 || HalfExtentVertsY == 0)
		return {};

	const FVector BoundPosGlobal = Landscape.GetActorTransform().TransformPositionNoScale(FVector(
		static_cast<double>(ClosestVertexX) * QuadSizeX,
		static_cast<double>(ClosestVertexY) * QuadSizeY, 0));

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent = FVector(
		static_cast<double>(HalfExtentVertsX) * QuadSizeX,
		static_cast<double>(HalfExtentVertsY) * QuadSizeY, SelectedHalfExtent.Z);

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

#if WITH_EDITOR
bool UAGX_HeightFieldBoundsComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	UWorld* World = GetWorld();
	if (World == nullptr || !World->IsPlayInEditor())
	{
		return SuperCanEditChange;
	}

	const FName Prop = InProperty->GetFName();
	if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_HeightFieldBoundsComponent, HalfExtent))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_HeightFieldBoundsComponent, bInfiniteBounds))
		return false;
	else
		return SuperCanEditChange;
}
#endif
