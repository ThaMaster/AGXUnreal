#include "Wire/AGX_WireHitProxies.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

// Unreal Engine includes.
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "AGX_WireHitProxies"
#undef LOCTEXT_NAMESPACE

IMPLEMENT_HIT_PROXY(HNodeProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HWinchLocationProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HWinchDirectionProxy, HComponentVisProxy);

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
	const UAGX_WireComponent* WinchOwner, EWireSide Side, const FTransform& LocalToWorld,
	FPrimitiveDrawInterface* PDI)
{
	const FAGX_WireWinch* Winch = WinchOwner->GetWinch(Side);
	if (Winch == nullptr)
	{
		return FVector::ZeroVector;
	}

	return DrawWinch(
		*Winch, LocalToWorld, new HWinchLocationProxy(WinchOwner, EWireSide::Begin),
		new HWinchDirectionProxy(WinchOwner, EWireSide::Begin), PDI);
}

FVector AGX_WireVisualization_helpers::DrawWinch(
	const UAGX_WireWinchComponent* Winch, FPrimitiveDrawInterface* PDI)
{
	return DrawWinch(
		Winch->WireWinch, Winch->GetComponentTransform(), new HWinchLocationProxy(Winch),
		new HWinchDirectionProxy(Winch), PDI);
}
