#include "Wire/AGX_WireHitProxies.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/AGX_WireUtilities.h"

// Unreal Engine includes.
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "AGX_WireHitProxies"
#undef LOCTEXT_NAMESPACE

IMPLEMENT_HIT_PROXY(HNodeProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HWinchLocationProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HWinchDirectionProxy, HComponentVisProxy);

namespace AGX_WireVisualization_helpers
{
	const USceneComponent* GetFirstValid(
		const USceneComponent* First, const USceneComponent* Second)
	{
		if (IsValid(First))
		{
			return First;
		}
		if (IsValid(Second))
		{
			return Second;
		}
		return nullptr;
	}
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const FAGX_WireWinch& Winch, const FAGX_WireWinchPose& WinchPose,
	HWinchLocationProxy* LocationProxy, HWinchDirectionProxy* DirectionProxy,
	FPrimitiveDrawInterface* PDI)
{
	FLinearColor Color = FLinearColor::Red;
	float HandleSize = 10.0f;

	// Render winch location marker.
	const FTransform& LocalToWorld = WinchPose.LocalToWorld;
	const FVector LocalLocation = WinchPose.LocalLocation;
	const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
	PDI->SetHitProxy(LocationProxy);
	PDI->DrawPoint(WorldLocation, Color, HandleSize, SDPG_Foreground);
	PDI->SetHitProxy(nullptr);

	// Render winch direction marker.
	const FRotator LocalRotation = WinchPose.LocalRotation;
	const FVector LocalDirection = LocalRotation.RotateVector(FVector::ForwardVector);
	const FVector WorldDirection = LocalToWorld.TransformVector(LocalDirection);
	const FVector WorldEndLocation = WorldLocation + (WorldDirection * 100.0f);
	PDI->SetHitProxy(DirectionProxy);
	PDI->DrawPoint(WorldEndLocation, Color, HandleSize, SDPG_Foreground);
	PDI->SetHitProxy(nullptr);

	// Draw a line between the two markers.
	PDI->DrawLine(WorldLocation, WorldEndLocation, Color, SDPG_Foreground);

	return WorldLocation;
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const UAGX_WireComponent& Wire, EWireSide Side, FPrimitiveDrawInterface* PDI)
{
	const FAGX_WireWinch* Winch = Wire.GetWinch(Side);
	checkf(
		Winch != nullptr,
		TEXT("DrawWinch called for a Wire Component that doesn't have a winch at the given side."));
	const FAGX_WireWinchPose WinchPose = FAGX_WireUtilities::GetWireWinchPose(Wire, Side);
	HWinchLocationProxy* LocationProxy = new HWinchLocationProxy(&Wire, Side);
	HWinchDirectionProxy* DirectionProxy = new HWinchDirectionProxy(&Wire, Side);
	return DrawWinch(*Winch, WinchPose, LocationProxy, DirectionProxy, PDI);
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const UAGX_WireWinchComponent& Winch, FPrimitiveDrawInterface* PDI)
{
	const FAGX_WireWinchPose WinchPose = FAGX_WireUtilities::GetWireWinchPose(Winch);
	HWinchLocationProxy* LocationProxy = new HWinchLocationProxy(&Winch);
	HWinchDirectionProxy* DirectionProxy = new HWinchDirectionProxy(&Winch);
	return DrawWinch(Winch.WireWinch, WinchPose, LocationProxy, DirectionProxy, PDI);
}

namespace AGX_WireVisualization_helpers
{
	FVector GetWidgetLocationLocation(const FAGX_WireWinchPose& Pose)
	{
		return Pose.LocalToWorld.TransformPosition(Pose.LocalLocation);
	}

	FVector GetWidgetLocationRotation(const FAGX_WireWinchPose& Pose)
	{
		const FVector WorldLocation = Pose.LocalToWorld.TransformPosition(Pose.LocalLocation);
		const FVector LocalDirection = Pose.LocalRotation.RotateVector(FVector::ForwardVector);
		const FVector WorldDirection = Pose.LocalToWorld.TransformVector(LocalDirection);
		const FVector WorldEndLocation = WorldLocation + (WorldDirection * 100.0f);
		return WorldEndLocation;
	}
}

bool AGX_WireVisualization_helpers::GetWidgetLocation(
	const FAGX_WireWinchPose& WinchPose, EWinchSide Side, FVector& OutLocation)
{
	switch (Side)
	{
		case EWinchSide::Location:
			OutLocation = GetWidgetLocationLocation(WinchPose);
			return true;
		case EWinchSide::Rotation:
		{
			OutLocation = GetWidgetLocationRotation(WinchPose);
			return true;
		}
		case EWinchSide::None:
			checkNoEntry();
			return false;
	}
	checkNoEntry();
	return false;
}

void AGX_WireVisualization_helpers::TransformWinch(
	FAGX_WireWinch& Winch, const FTransform& WinchToWorld, EWinchSide Side,
	const FVector& DeltaTranslate, const FRotator& DeltaRotate)
{
	switch (Side)
	{
		case EWinchSide::Location:
			TransformWinchLocation(Winch, WinchToWorld, DeltaTranslate, DeltaRotate);
			return;
		case EWinchSide::Rotation:
			TransformWinchRotation(Winch, WinchToWorld, DeltaTranslate);
			break;
		case EWinchSide::None:
			// Nothing to do here.
			break;
	}
}

void AGX_WireVisualization_helpers::TransformWinchLocation(
	FAGX_WireWinch& Winch, const FTransform& WinchToWorld, const FVector& DeltaTranslate,
	const FRotator& DeltaRotate)
{
	/*
	 * Direct winch translation is only allowed at edit time so it is always the non-sim location
	 * and rotation that is updated.
	 */

	if (!DeltaTranslate.IsZero())
	{
		const FVector LocalTranslate = WinchToWorld.InverseTransformVector(DeltaTranslate);
		Winch.Location += LocalTranslate;
	}

	if (!DeltaRotate.IsZero())
	{
		const FVector Direction = Winch.Rotation.RotateVector(FVector::ForwardVector);
		const FVector WorldDirection = WinchToWorld.TransformVector(Direction);
		const FVector NewWorldDirection = DeltaRotate.RotateVector(WorldDirection);
		const FVector NewLocalDirection = WinchToWorld.InverseTransformVector(NewWorldDirection);
		const FRotator NewRotation =
			FQuat::FindBetween(FVector::ForwardVector, NewLocalDirection).Rotator();
		Winch.Rotation = NewRotation;
	}
}

void AGX_WireVisualization_helpers::TransformWinchRotation(
	FAGX_WireWinch& Winch, const FTransform& WinchToWorld, const FVector& DeltaTranslate)
{
	/*
	 * Direct winch rotation is only allowed at edit time so it is always the non-sim rotation that
	 * is updated.
	 */

	const FVector LocalBeginLocation = Winch.Location;
	const FRotator Rotation = Winch.Rotation;
	const FVector LocalDirection = Rotation.RotateVector(FVector::ForwardVector);
	const FVector LocalEndLocation = LocalBeginLocation + (LocalDirection * 100.0f);
	const FVector LocalTranslate = WinchToWorld.InverseTransformVector(DeltaTranslate);
	const FVector NewLocalEndLocation = LocalEndLocation + LocalTranslate;
	const FVector NewDirection = NewLocalEndLocation - LocalBeginLocation;
	const FRotator NewRotation = FQuat::FindBetween(FVector::ForwardVector, NewDirection).Rotator();
	Winch.Rotation = NewRotation;
}
