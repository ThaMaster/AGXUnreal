#include "Wire/AGX_WireHitProxies.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

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

const FTransform& AGX_WireVisualization_helpers::GetOwnedWinchLocalToWorld(
	const UAGX_WireComponent& Wire, const FAGX_WireWinch& Winch)
{
	const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment();
	const USceneComponent* Component = GetFirstValid(Body, &Wire);
	if (Component == nullptr)
	{
		return FTransform::Identity;
	}
	return Component->GetComponentTransform();
}

const FTransform& AGX_WireVisualization_helpers::GetOwnedWinchLocalToWorld(
	const UAGX_WireComponent& Wire, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetOwnedWinchLocalToWorld(Wire, Wire.OwnedBeginWinch);
		case EWireSide::End:
			return GetOwnedWinchLocalToWorld(Wire, Wire.OwnedEndWinch);
		case EWireSide::None:
			return Wire.GetComponentTransform();
	}
	return Wire.GetComponentTransform();
}

const FTransform& AGX_WireVisualization_helpers::GetWinchLocalToWorld(
	const UAGX_WireComponent& Wire, EWireSide Side)
{
	switch (Wire.GetWinchOwnerType(Side))
	{
		case EWireWinchOwnerType::Wire:
			return GetOwnedWinchLocalToWorld(Wire, Side);
		case EWireWinchOwnerType::WireWinch:
			return Wire.GetWinchComponent(Side)->GetComponentTransform();
		case EWireWinchOwnerType::Other:
			// We know nothing of these Wire Winches, so their location and rotation must be
			// in the world coordinate system at all times.
			return FTransform::Identity;
		case EWireWinchOwnerType::None:
			return FTransform::Identity;
	}
	return FTransform::Identity;
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const FAGX_WireWinch& Winch, const FTransform& LocalToWorld, HWinchLocationProxy* LocationProxy,
	HWinchDirectionProxy* DirectionProxy, FPrimitiveDrawInterface* PDI)
{
	FLinearColor Color = FLinearColor::Red;
	float HandleSize = 10.0f;

	const FVector LocalLocation = Winch.Location;
	const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
	PDI->SetHitProxy(LocationProxy);
	PDI->DrawPoint(WorldLocation, Color, HandleSize, SDPG_Foreground);
	PDI->SetHitProxy(nullptr);

	const FRotator Rotation = Winch.Rotation;
	const FVector LocalDirection = Rotation.RotateVector(FVector::ForwardVector);
	const FVector WorldDirection = LocalToWorld.TransformVector(LocalDirection);
	const FVector WorldEndLocation = WorldLocation + (WorldDirection * 100.0f);
	PDI->SetHitProxy(DirectionProxy);
	PDI->DrawPoint(WorldEndLocation, Color, HandleSize, SDPG_Foreground);
	PDI->SetHitProxy(nullptr);

	PDI->DrawLine(WorldLocation, WorldEndLocation, Color, SDPG_Foreground);

	return WorldLocation;
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const UAGX_WireComponent& Wire, EWireSide Side, FPrimitiveDrawInterface* PDI)
{
	const FTransform& WinchToWorld = GetWinchLocalToWorld(Wire, Side);
	const FAGX_WireWinch* Winch = Wire.GetWinch(Side);
	if (Winch == nullptr)
	{
		return FVector::ZeroVector;
	}
	return DrawWinch(
		*Winch, WinchToWorld, new HWinchLocationProxy(&Wire, Side),
		new HWinchDirectionProxy(&Wire, Side), PDI);
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const UAGX_WireWinchComponent& Winch, FPrimitiveDrawInterface* PDI)
{
	return DrawWinch(
		Winch.WireWinch, Winch.GetComponentTransform(), new HWinchLocationProxy(&Winch),
		new HWinchDirectionProxy(&Winch), PDI);
}

namespace AGX_WireVisualization_helpers
{
	FVector GetWinchLocationWidgetLocation(
		const FAGX_WireWinch& Winch, const FTransform& WinchToWorld)
	{
		const FVector LocalLocation = Winch.Location;
		const FVector WorldLocation = WinchToWorld.TransformPosition(LocalLocation);
		return WorldLocation;
	}

	FVector GetWinchRotationWidgetLocation(
		const FAGX_WireWinch& Winch, const FTransform& WinchToWorld)
	{
		const FVector LocalLocation = Winch.Location;
		const FVector WorldLocation = WinchToWorld.TransformPosition(LocalLocation);
		const FRotator Rotation = Winch.Rotation;
		const FVector LocalDirection = Rotation.RotateVector(FVector::ForwardVector);
		const FVector WorldDirection = WinchToWorld.TransformVector(LocalDirection);
		const FVector WorldEndLocation = WorldLocation + (WorldDirection * 100.0f);
		return WorldEndLocation;
	}
}

bool AGX_WireVisualization_helpers::GetWidgetLocation(
	const FAGX_WireWinch& Winch, const FTransform& WinchToWorld, EWinchSide WinchSide,
	FVector& OutLocation)
{
	switch (WinchSide)
	{
		case EWinchSide::Location:
			OutLocation = GetWinchLocationWidgetLocation(Winch, WinchToWorld);
			return true;
		case EWinchSide::Rotation:
			OutLocation = GetWinchRotationWidgetLocation(Winch, WinchToWorld);
			return true;
		case EWinchSide::None:
			return false;
	}
	return false;
}

bool AGX_WireVisualization_helpers::GetWidgetLocation(
	const UAGX_WireComponent& Wire, EWireSide WireSide, EWinchSide WinchSide, FVector& OutLocation)
{
	const FTransform& WinchToWorld = GetWinchLocalToWorld(Wire, WireSide);
	const FAGX_WireWinch* Winch = Wire.GetWinch(WireSide);
	if (Winch == nullptr)
	{
		return false;
	}
	return GetWidgetLocation(*Winch, WinchToWorld, WinchSide, OutLocation);
}

bool AGX_WireVisualization_helpers::GetWidgetLocation(
	const UAGX_WireWinchComponent& Winch, EWinchSide WinchSide, FVector& OutLocation)
{
	const FTransform& WinchToWorld = Winch.GetComponentTransform();
	const FAGX_WireWinch& WireWinch = Winch.WireWinch;
	return GetWidgetLocation(WireWinch, WinchToWorld, WinchSide, OutLocation);
}

void AGX_WireVisualization_helpers::TransformWinchLocation(
	FAGX_WireWinch& Winch, const FTransform& WinchToWorld, const FVector& DeltaTranslate,
	const FRotator& DeltaRotate)
{
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
