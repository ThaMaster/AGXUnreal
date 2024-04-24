// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGXUnrealBarrier.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Wire/AGX_WireInstanceData.h"
#include "Wire/AGX_WireNode.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "CoreGlobals.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

// Standard library includes.
#include <algorithm>
#include <tuple>

#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif

	// Add a pair of default nodes to make initial editing easier.
	AddNodeAtLocation(FVector::ZeroVector);
	AddNodeAtLocation(FVector(100.0f, 0.0f, 0.0f));

	// Setup default visuals.
	static const TCHAR* WireMatAssetPath =
		TEXT("Material'/AGXUnreal/Wire/MI_GrayWire.MI_GrayWire'");
	RenderMaterial = FAGX_ObjectUtilities::GetAssetFromPath<UMaterialInterface>(WireMatAssetPath);
}

void UAGX_WireComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}
	Radius = InRadius;

	UpdateVisuals();
}

void UAGX_WireComponent::SetMinSegmentLength(float InMinSegmentLength)
{
	if (HasNative())
	{
		const float ResolutionPerUnitLength = 1.0f / InMinSegmentLength;
		NativeBarrier.SetResolutionPerUnitLength(ResolutionPerUnitLength);
	}
	MinSegmentLength = InMinSegmentLength;
}

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedBeginWinch_BP()
{
	return {&OwnedBeginWinch};
}

bool UAGX_WireComponent::HasOwnedBeginWinch() const
{
	return BeginWinchType == EWireWinchOwnerType::Wire;
}

bool UAGX_WireComponent::HasBeginWinchComponent() const
{
	return GetBeginWinchComponent() != nullptr;
}

void UAGX_WireComponent::SetBeginWinchComponent(UAGX_WireWinchComponent* Winch)
{
	if (Winch == nullptr)
	{
		BeginWinchComponent = FComponentReference();
		return;
	}

	BeginWinchComponent.OtherActor = Winch->GetOwner();
	BeginWinchComponent.ComponentProperty = Winch->GetFName();
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetBeginWinchComponent()
{
	return FAGX_ObjectUtilities::Get<UAGX_WireWinchComponent>(BeginWinchComponent, GetOwner());
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetBeginWinchComponent() const
{
	return FAGX_ObjectUtilities::Get<const UAGX_WireWinchComponent>(
		BeginWinchComponent, GetOwner());
}

FAGX_WireWinch* UAGX_WireComponent::GetBeginWinchComponentWinch()
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetBeginWinchComponentWinch());
}

const FAGX_WireWinch* UAGX_WireComponent::GetBeginWinchComponentWinch() const
{
	const UAGX_WireWinchComponent* WinchComponent = GetBeginWinchComponent();
	if (WinchComponent == nullptr)
	{
		return nullptr;
	}
	return &WinchComponent->WireWinch;
}

FAGX_WireWinchRef UAGX_WireComponent::GetBeginWinchComponentWinch_BP()
{
	return {GetBeginWinchComponentWinch()};
}

void UAGX_WireComponent::SetBorrowedBeginWinch(FAGX_WireWinchRef Winch)
{
	BorrowedBeginWinch = Winch.Winch;
}

bool UAGX_WireComponent::HasBeginWinch() const
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return true;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent();
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch != nullptr;
		case EWireWinchOwnerType::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot determine presence of "
			 "begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetBeginWinch()
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedBeginWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent() ? GetBeginWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch != nullptr ? BorrowedBeginWinch : nullptr;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot get begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetBeginWinch() const
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedBeginWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent() ? GetBeginWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot get begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()))
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetBeginWinch_BP()
{
	return {GetBeginWinch()};
}

bool UAGX_WireComponent::AttachOwnedBeginWinch()
{
	return AttachOwnedWinch(EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinch(UAGX_WireWinchComponent* Winch)
{
	return AttachWinch(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinch(FAGX_WireWinchRef Winch)
{
	return AttachWinch(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinchToComponent(UAGX_WireWinchComponent* Winch)
{
	return AttachWinchToComponent(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinchToOther(FAGX_WireWinchRef Winch)
{
	return AttachWinchToOther(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::DetachBeginWinch()
{
	return DetachWinch(EWireSide::Begin);
}

/*
 * End Winch.
 */

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedEndWinch_BP()
{
	return {&OwnedEndWinch};
}

bool UAGX_WireComponent::HasOwnedEndWinch() const
{
	return EndWinchType == EWireWinchOwnerType::Wire;
}

bool UAGX_WireComponent::HasEndWinchComponent() const
{
	return GetEndWinchComponent() != nullptr;
}

void UAGX_WireComponent::SetEndWinchComponent(UAGX_WireWinchComponent* Winch)
{
	if (Winch == nullptr)
	{
		EndWinchComponent = FComponentReference();
		return;
	}

	EndWinchComponent.OtherActor = Winch->GetOwner();
	EndWinchComponent.ComponentProperty = Winch->GetFName();
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetEndWinchComponent()
{
	return FAGX_ObjectUtilities::Get<UAGX_WireWinchComponent>(EndWinchComponent, GetOwner());
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetEndWinchComponent() const
{
	return FAGX_ObjectUtilities::Get<const UAGX_WireWinchComponent>(EndWinchComponent, GetOwner());
}

FAGX_WireWinch* UAGX_WireComponent::GetEndWinchComponentWinch()
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetEndWinchComponentWinch());
}

FAGX_WireWinchRef UAGX_WireComponent::GetEndWinchComponentWinch_BP()
{
	return {GetEndWinchComponentWinch()};
}

const FAGX_WireWinch* UAGX_WireComponent::GetEndWinchComponentWinch() const
{
	const UAGX_WireWinchComponent* WinchComponent = GetEndWinchComponent();
	if (WinchComponent == nullptr)
	{
		return nullptr;
	}
	return &WinchComponent->WireWinch;
}

void UAGX_WireComponent::SetBorrowedEndWinch(FAGX_WireWinchRef Winch)
{
	BorrowedEndWinch = Winch.Winch;
}

bool UAGX_WireComponent::HasEndWinch() const
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return true;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent();
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch != nullptr;
		case EWireWinchOwnerType::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot determine presence of end "
			 "winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetEndWinch()
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedEndWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent() ? GetEndWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch != nullptr ? BorrowedEndWinch : nullptr;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot get end winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetEndWinch() const
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedEndWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent() ? GetEndWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot get end winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetEndWinch_BP()
{
	return {GetEndWinch()};
}

bool UAGX_WireComponent::AttachOwnedEndWinch()
{
	return AttachOwnedWinch(EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinch(UAGX_WireWinchComponent* Winch)
{
	return AttachWinch(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinch(FAGX_WireWinchRef Winch)
{
	return AttachWinch(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinchToComponent(UAGX_WireWinchComponent* Winch)
{
	return AttachWinchToComponent(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinchToOther(FAGX_WireWinchRef Winch)
{
	return AttachWinchToOther(Winch, EWireSide::End);
}

bool UAGX_WireComponent::DetachEndWinch()
{
	return DetachWinch(EWireSide::End);
}

/*
 * Side-agnostic winch.
 */

void UAGX_WireComponent::SetWinchType(EWireWinchOwnerType Type, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			BeginWinchType = Type;
			return;
		case EWireSide::End:
			EndWinchType = Type;
			return;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Winch Type for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Type for Wire '%s' in '%s'. Cannot set winch "
			 "type."),
		*GetName(), *GetLabelSafe(GetOwner()));
}

EWireWinchOwnerType UAGX_WireComponent::GetWinchType(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BeginWinchType;
		case EWireSide::End:
			return EndWinchType;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Get Winch Type for Wire '%s' in '%s'."), *GetName(),
				*GetNameSafe(GetOwner()));
			return EWireWinchOwnerType::None;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Type for Wire '%s' in '%s'. Cannot determine "
			 "winch type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return EWireWinchOwnerType::None;
}

FAGX_WireWinch* UAGX_WireComponent::GetOwnedWinch(EWireSide Side)
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetOwnedWinch(Side));
}

const FAGX_WireWinch* UAGX_WireComponent::GetOwnedWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return &OwnedBeginWinch;
		case EWireSide::End:
			return &OwnedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Owned Winch for Wire '%s' in '%s'. Cannot determine "
			 "owned winch to return."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedWinch_BP(EWireSide Side)
{
	return {GetOwnedWinch(Side)};
}

bool UAGX_WireComponent::SetBorrowedWinch(FAGX_WireWinchRef Winch, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			SetBorrowedBeginWinch(Winch);
			return true;
		case EWireSide::End:
			SetBorrowedEndWinch(Winch);
			return true;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Borrowed Winch for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Borrowed Winch for Wire '%s' in '%s'. Cannot set "
			 "winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

bool UAGX_WireComponent::HasWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return HasBeginWinch();
		case EWireSide::End:
			return HasEndWinch();
		case EWireSide::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Has Winch for Wire '%s' in '%s'. Cannot determine winch "
			 "presence."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch for Wire '%s' in '%s'. Cannot get winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch for Wire '%s' in '%s'. Cannot get winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetWinch_BP(EWireSide Side)
{
	return {GetWinch(Side)};
}

bool UAGX_WireComponent::AttachOwnedWinch(EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachOwnedWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	checkf(GetOwnedWinch(Side) != nullptr, TEXT("Did not get a winch despite valid side."));
	FAGX_WireWinch& Winch = *GetOwnedWinch(Side);
	SetWinchType(EWireWinchOwnerType::Wire, Side);
	if (HasNative())
	{
		FAGX_WireUtilities::ComputeSimulationPlacement(*this, Winch);
		FWireWinchBarrier* Barrier = Winch.GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to it's owned %s winch because the AGX "
					 "Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "owned Wire Winch. See LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, EWireSide::Begin);
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinch(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	if (Winch == nullptr)
	{
		SetWinchType(EWireWinchOwnerType::None, Side);
		SetWinchComponent(nullptr, Side);
		if (HasNative())
		{
			NativeBarrier.Detach(Side == EWireSide::Begin);
		}
		return true;
	}

	SetWinchType(EWireWinchOwnerType::WireWinch, Side);
	SetWinchComponent(Winch, Side);

	if (HasNative())
	{
		FWireWinchBarrier* Barrier = Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to the Wire Winch '%s' in '%s' because the "
					 "AGX Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()), *Winch->GetName(),
				*GetNameSafe(Winch->GetOwner()));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		const bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "Wire Winch Component. See LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinch(FAGX_WireWinchRef Winch, EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	if (!Winch.IsValid())
	{
		SetWinchType(EWireWinchOwnerType::None, Side);
		SetBorrowedWinch({nullptr}, Side);
		if (HasNative())
		{
			NativeBarrier.Detach(Side == EWireSide::Begin);
		}
		return true;
	}

	SetWinchType(EWireWinchOwnerType::Other, Side);
	SetBorrowedWinch(Winch, Side);
	if (HasNative())
	{
		FAGX_WireUtilities::ComputeSimulationPlacement(*Winch.Winch);
		FWireWinchBarrier* Barrier = Winch.Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to custom Wire Winch because the AGX "
					 "Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		const bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "custom Wire Winch. See LogAGXDynamics for details."),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side), *GetName(),
				*GetNameSafe(GetOwner()));
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinchToComponent(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	return AttachWinch(Winch, Side);
}

bool UAGX_WireComponent::AttachWinchToOther(FAGX_WireWinchRef Winch, EWireSide Side)
{
	return AttachWinch(Winch, Side);
}

bool UAGX_WireComponent::DetachWinch(EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to DetachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	SetWinchType(EWireWinchOwnerType::None, Side);
	if (HasNative())
	{
		bool bDetached = NativeBarrier.Detach(Side == EWireSide::Begin);
		if (!bDetached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not detach %s side. See "
					 "LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			return false;
		}
	}
	return true;
}

bool UAGX_WireComponent::SetWinchOwnerType(EWireSide Side, EWireWinchOwnerType Type)
{
	switch (Side)
	{
		case EWireSide::Begin:
			BeginWinchType = Type;
			return true;
		case EWireSide::End:
			EndWinchType = Type;
			return true;
		case EWireSide::None:
			return true;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Owner Type for Wire '%s' in '%s'. Cannot set "
			 "winch owner type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

EWireWinchOwnerType UAGX_WireComponent::GetWinchOwnerType(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BeginWinchType;
		case EWireSide::End:
			return EndWinchType;
		case EWireSide::None:
			return EWireWinchOwnerType::None;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Owner Type for Wire '%s' in '%s'. Cannot get "
			 "winch owner type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return EWireWinchOwnerType::None;
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetWinchComponent(EWireSide Side)
{
	return const_cast<UAGX_WireWinchComponent*>(
		const_cast<const UAGX_WireComponent*>(this)->GetWinchComponent(Side));
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetWinchComponent(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinchComponent();
		case EWireSide::End:
			return GetEndWinchComponent();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Component for Wire '%s' in '%s'. Cannot get "
			 "winch component."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

bool UAGX_WireComponent::SetWinchComponent(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			SetBeginWinchComponent(Winch);
			return true;
		case EWireSide::End:
			SetEndWinchComponent(Winch);
			return true;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Winch Component for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Component for Wire '%s' in '%s'. Cannot set "
			 "winch component."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FComponentReference* UAGX_WireComponent::GetWinchComponentReference(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return &BeginWinchComponent;
		case EWireSide::End:
			return &EndWinchComponent;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed Get Winch Component Reference for Wire '%s' in '%s'. Cannot "
			 "get winch component reference."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinch* UAGX_WireComponent::GetBorrowedWinch(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BorrowedBeginWinch;
		case EWireSide::End:
			return BorrowedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Borrowed Winch for Wire '%s' in '%s'. Cannot get "
			 "borrowed winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetBorrowedWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BorrowedBeginWinch;
		case EWireSide::End:
			return BorrowedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Borrowed Winch for Wire '%s' in '%s'. Cannot get "
			 "borrowed winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

namespace AGX_WireComponent_helpers
{
	void PrintNodeModifiedAlreadyInitializedWarning()
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Route node modification to already initialized wire. This route node will be "
				 "ignored."));
	}
}

FWireRoutingNode& UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode)
{
	int32 _;
	return AddNode(InNode, _);
}

// TODO This code will need to change with the introduction of frame in routing nodes.
//
// All FWireRoutingNodes added in these Add-functions must have an Owning Actor. If it doesn't have
// one already then set GetTypedOuter<AActor>().
FWireRoutingNode& UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode, int32& OutIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Add(InNode);
	FWireRoutingNode& NewNode = RouteNodes.Last();
	FAGX_ObjectUtilities::SetIfNullptr(NewNode.Frame.Parent.OwningActor, GetTypedOuter<AActor>());
	OutIndex = RouteNodes.Num() - 1;
	return NewNode;
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocation(FVector InLocation)
{
	int32 _;
	return AddNodeAtLocation(InLocation, _);
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocation(FVector InLocation, int32& OutIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Add(FWireRoutingNode(InLocation));
	FWireRoutingNode& NewNode = RouteNodes.Last();
	FAGX_ObjectUtilities::SetIfNullptr(NewNode.Frame.Parent.OwningActor, GetTypedOuter<AActor>());

	UE_LOG(
		LogAGX, Warning, TEXT("Created node at address %p at location %s"), &NewNode,
		*InLocation.ToString());

	OutIndex = RouteNodes.Num() - 1;
	return NewNode;
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Insert(InNode, InIndex);
	FWireRoutingNode& NewNode = RouteNodes[InIndex];
	FAGX_ObjectUtilities::SetIfNullptr(NewNode.Frame.Parent.OwningActor, GetTypedOuter<AActor>());
	return NewNode;
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocationAtIndex(
	const FVector& InLocation, int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Insert(FWireRoutingNode(InLocation), InIndex);
	FWireRoutingNode& NewNode = RouteNodes[InIndex];
	FAGX_ObjectUtilities::SetIfNullptr(NewNode.Frame.Parent.OwningActor, GetTypedOuter<AActor>());
	return NewNode;
}

void UAGX_WireComponent::SetNode(const int32 InIndex, const FWireRoutingNode InNode)
{
	UE_LOG(
		LogAGX, Warning, TEXT("Setting node at address %p at location %s"), &InNode,
		*InNode.Frame.LocalLocation.ToString());
	if (!RouteNodes.IsValidIndex(InIndex))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Out-of-bounds index %d for route nodes array was passed to Set Node in Wire "
				 "Component "
				 "'%s' in Actor '%s'"),
			InIndex, *GetName(), *GetLabelSafe(GetOwner()));
		return;
	}
	RouteNodes[InIndex] = InNode;
}

void UAGX_WireComponent::RemoveNode(int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.RemoveAt(InIndex);
}

void UAGX_WireComponent::SetNodeLocalLocation(int32 InIndex, const FVector& InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
	RouteNodes[InIndex].Frame.LocalLocation = InLocation;
#else
	RouteNodes[InIndex].Location = InLocation;
#endif
}

void UAGX_WireComponent::SetNodeLocation(int32 InIndex, const FVector& InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
	USceneComponent* Parent = RouteNodes[InIndex].Frame.GetParentComponent();
	if (Parent == nullptr)
	{
		// No parent means the LocalLocation is relative to the Wire Component and thus InComponent
		// can be used as-is.
		RouteNodes[InIndex].Frame.LocalLocation = InLocation;
		return;
	}

	// Compute a local location relative to the parent that as the same world location as
	// InLocation in the Wire Component.
	const FTransform& ParentTransform = Parent->GetComponentTransform();
	const FTransform& WireTransform = GetComponentTransform();
	const FTransform& WireToParent = WireTransform.GetRelativeTransform(ParentTransform);
	const FVector LocationInParent = WireToParent.TransformPosition(InLocation);
	RouteNodes[InIndex].Frame.LocalLocation = LocationInParent;
#else
	RouteNodes[InIndex].Location = InLocation;
#endif
}

bool UAGX_WireComponent::IsLumpedNode(const FAGX_WireNode& Node)
{
	if (!HasNative() || !Node.HasNative())
	{
		return false;
	}

	return NativeBarrier.IsLumpedNode(*Node.GetNative());
}

bool UAGX_WireComponent::IsInitialized() const
{
	if (!HasNative())
	{
		return false;
	}
	return NativeBarrier.IsInitialized();
}

double UAGX_WireComponent::GetRestLength() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRestLength();
	}
	if (RouteNodes.Num() <= 1)
	{
		return 0.0;
	}

#if AGX_WIRE_ROUTE_NODE_USE_FRAME
	double Length {0.0};
	FVector PreviousLocation = RouteNodes[0].Frame.GetWorldLocation(*this);
	for (int32 I = 1; I < RouteNodes.Num(); ++I)
	{
		const FVector Location = RouteNodes[I].Frame.GetWorldLocation(*this);
		Length += FVector::Distance(PreviousLocation, Location);
		PreviousLocation = Location;
	}
#else
	double Length = 0.0;
	for (int32 I = 1; I < RouteNodes.Num(); ++I)
	{
		Length += FVector::Distance(RouteNodes[I - 1].Location, RouteNodes[I].Location);
	}
#endif
	return Length;
}

float UAGX_WireComponent::GetRestLength_BP() const
{
	return static_cast<float>(GetRestLength());
}

double UAGX_WireComponent::GetMass() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMass();
	}
	if (ShapeMaterial == nullptr)
	{
		/// @note Can we find the density that AGX Dynamics will use for wires that don't have an
		/// explicit material set?
		return 0.0;
	}
	const double Area = PI * Radius * Radius; // Assume circular cross-section.
	const double Length = GetRestLength();
	const double Density = ShapeMaterial->Bulk.Density;
	const double Mass = Area * Length * Density;
	return Mass;
}

float UAGX_WireComponent::GetMass_BP() const
{
	return static_cast<float>(GetMass());
}

double UAGX_WireComponent::GetTension() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetTension();
}

float UAGX_WireComponent::GetTension_BP() const
{
	return static_cast<float>(GetTension());
}

bool UAGX_WireComponent::HasRenderNodes() const
{
	if (!HasNative())
	{
		return false;
	}
	return !NativeBarrier.GetRenderListEmpty();
}

bool UAGX_WireComponent::GetRenderListEmpty() const
{
	if (!HasNative())
	{
		return true;
	}
	return NativeBarrier.GetRenderListEmpty();
}

FAGX_WireRenderIterator UAGX_WireComponent::GetRenderBeginIterator() const
{
	if (!HasNative())
	{
		return {};
	}
	return {NativeBarrier.GetRenderBeginIterator()};
}

FAGX_WireRenderIterator UAGX_WireComponent::GetRenderEndIterator() const
{
	if (!HasNative())
	{
		return {};
	}
	return {NativeBarrier.GetRenderEndIterator()};
}

TArray<FVector> UAGX_WireComponent::GetRenderNodeLocations() const
{
	TArray<FVector> Result;
	for (auto It = GetRenderBeginIterator(), End = GetRenderEndIterator(); It != End; It.Inc())
	{
		const FAGX_WireNode Node = It.Get();
		Result.Add(Node.GetWorldLocation());
	}
	return Result;
}

#if WITH_EDITOR

void UAGX_WireComponent::OnRouteNodeParentMoved(
	USceneComponent* Component, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("Wire Component '%s' (%p) in '%s' (%p) got parent moved callback from '%s' (%p) in "
			 "'%s' (%p)."),
		*GetName(), this, *GetLabelSafe(GetOwner()), GetOwner(), *Component->GetName(), Component,
		*GetLabelSafe(Component->GetOwner()), Component->GetOwner());

	// If this is a callback from a parent we are no longer a child of, then unsubscribe.
	// TODO Is there a better way to avoid keeping callbacks laying around in the parents?
	// I would like to do this in PreEditChange.
	FWireRoutingNode* Node =
		RouteNodes.FindByPredicate([Component](const FWireRoutingNode& Node)
								   { return Node.Frame.GetParentComponent() == Component; });
	if (Node == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("  The Component is not any node's parent. Removing."));
		// Component is not the parent of any node, unsubscribe.
		const FParentDelegate* Handle = DelegateHandles.Find(Component);
		if (Handle != nullptr && Handle->Parent.IsValid())
		{
			Component->TransformUpdated.Remove(Handle->DelegateHandle);
			DelegateHandles.Remove(Component);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("  The Component wasn't in the Delegate Handles list. Why did we get a "
					 "callback? Can we remove us from the delegate without a handle?"));
		}
		return;
	}

	// At least one Routing Node has the moved Component as its parent.
	UE_LOG(LogAGX, Warning, TEXT("	Parent moved callback is updating wire visuals."));
	UpdateVisuals();
}

void UAGX_WireComponent::OnRouteNodeParentReplaced(
	const FCoreUObjectDelegates::FReplacementObjectMap& OldToNew)
{
	UE_LOG(LogAGX, Warning, TEXT("UAGX_WireComponent::OnRouteNodeParentReplaced for %p"), this);

	UE_LOG(LogAGX, Warning, TEXT("  Old-to-New:"));
	for (const TTuple<UObject*, UObject*>& Entry : OldToNew)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("    Old = %p -> New = %p  Type = %s"), Entry.Key, Entry.Value,
			*Entry.Key->GetClass()->GetName());
	}

	// Build reverse look-up table. By the time we get here the replacement has already happened
	// in the owning Actor so the call to Parent.GetComponent below will find the new object, not
	// the old. In order to quickly determine if the object is replaced or untouched by the
	// Blueprint Reconstruction we need to be able to determine if it is in the OldToNew map on the
	// value side.
	FCoreUObjectDelegates::FReplacementObjectMap NewToOld;
	NewToOld.Reserve(OldToNew.Num());
	for (const TTuple<UObject*, UObject*>& Entry : OldToNew)
	{
		// Not all Components are replaced, we sometimes get nullptr in the value-part of the entry
		// Not sure what that means. Did the object disappear? Was the object not replaced so the
		// old pointer is still relevant? In any case, don't want to map nullptr to some random
		// Component, so skipping those entries.
		if (Entry.Value != nullptr)
		{
			NewToOld.Add(Entry.Value, Entry.Key);
		}
	}

	// Any changes made to the callback setup is a sign that we may need to update the wire
	// rendering positions. For example if a Scene Component was renamed to either become or stop
	// being a frame parent to any routing node.
	bool bNeedUpdateVisuals {false};

	for (FWireRoutingNode& Node : RouteNodes)
	{
		// Find our current (new) parent and the old parent that the new parent replaces.
		// If the parent wasn't replaced at all then OldParent will be nullptr and NewParent will
		// point to an old object.
		USceneComponent* NewParent = Node.Frame.Parent.GetComponent<USceneComponent>();
		USceneComponent* OldParent = Cast<USceneComponent>(NewToOld.FindRef(NewParent));
		UE_LOG(
			LogAGX, Warning, TEXT("Node is updating parent %p (isValid %d) to %p (isValid %d)."),
			OldParent, IsValid(OldParent), NewParent, IsValid(NewParent));
		if (!IsValid(NewParent) && !IsValid(OldParent))
		{
			continue;
		}

		UE_LOG(
			LogAGX, Warning, TEXT("  Wire route node parent replaced. Old = %p New %p "), OldParent,
			NewParent);

		// Remove ourselves from the old parent.
		if (OldParent != nullptr)
		{
			UE_LOG(LogAGX, Warning, TEXT("    Removing callbacks from %p"), OldParent);
			OldParent->TransformUpdated.RemoveAll(this);
			DelegateHandles.Remove(OldParent);
			bNeedUpdateVisuals = true;
		}

		// Add ourselves to the new parent, unless the new parent is actually an old object, i.e.
		// wasn't replaced, in which case we already have an entry for it in Delegate Handles.
		if (IsValid(NewParent) /* && !DelegateHandles.Contains(NewParent) */)
		{
			if (!DelegateHandles.Contains(NewParent))
			{
				UE_LOG(LogAGX, Warning, TEXT("    Adding callback to %p"), NewParent);
				DelegateHandles.Add(
					NewParent, {NewParent, NewParent->TransformUpdated.AddUObject(
											   this, &UAGX_WireComponent::OnRouteNodeParentMoved)});
				bNeedUpdateVisuals = true;
			}
			else
			{
				UE_LOG(LogAGX, Warning, TEXT("    Already have a callback in %p"), NewParent);
			}
		}
	}

	if (bNeedUpdateVisuals)
	{
		UpdateVisuals();
	}
}

#endif

void UAGX_WireComponent::MarkVisualsDirty()
{
	UpdateVisuals();
}

void UAGX_WireComponent::CopyFrom(const FWireBarrier& Barrier)
{
	Radius = Barrier.GetRadius();
	MinSegmentLength = 1.0f / Barrier.GetResolutionPerUnitLength();
	LinearVelocityDamping = static_cast<float>(Barrier.GetLinearVelocityDamping());
	bCanCollide = Barrier.GetEnableCollisions();

	for (const FName& Group : Barrier.GetCollisionGroups())
	{
		AddCollisionGroup(Group);
	}

	const FMergeSplitPropertiesBarrier Msp =
		FMergeSplitPropertiesBarrier::CreateFrom(*const_cast<FWireBarrier*>(&Barrier));
	if (Msp.HasNative())
	{
		MergeSplitProperties.CopyFrom(Msp);
	}

	ImportGuid = Barrier.GetGuid();

	// Physical material, winches, and route nodes not set here since this is a pure data copy. For
	// AGX Dynamics archive import these are set by AGX_ArchiveImporterHelper.
}

bool UAGX_WireComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_WireComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_WireComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));

	if (HasNative())
	{
		MergeSplitProperties.BindBarrierToOwner(*GetNative());
	}
}

FWireBarrier* UAGX_WireComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Wire '%s' in '%s' was made but "
					 "we are in the middle of a Blueprint Reconstruction and the requested instance"
					 "has not yet been restored. The instance cannot be returned, which may lead to"
					 "incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return &NativeBarrier;
}

FWireBarrier* UAGX_WireComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FWireBarrier* UAGX_WireComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_WireComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UE_LOG(LogAGX, Warning, TEXT("UAGX_WireComponent::PostInitProperties"));
	AActor* OwningActor = GetTypedOuter<AActor>();
	OwnedBeginWinch.BodyAttachment.OwningActor = OwningActor;
	OwnedEndWinch.BodyAttachment.OwningActor = OwningActor;
	for (FWireRoutingNode& Node : RouteNodes)
	{
		Node.Frame.Parent.OwningActor = OwningActor;
	}

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

void UAGX_WireComponent::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogAGX, Warning, TEXT("UAGX_WireComponent::PostLoad"));

	AActor* OwningActor = GetTypedOuter<AActor>();

	FAGX_ObjectUtilities::SetIfNullptr(OwnedBeginWinch.BodyAttachment.OwningActor, OwningActor);
	FAGX_ObjectUtilities::SetIfNullptr(OwnedBeginWinch.BodyAttachment.OwningActor, OwningActor);
	FAGX_ObjectUtilities::SetIfNullptr(OwnedEndWinch.BodyAttachment.OwningActor, OwningActor);
	for (FWireRoutingNode& Node : RouteNodes)
	{
		FAGX_ObjectUtilities::SetIfNullptr(Node.Frame.Parent.OwningActor, OwningActor);
	}

#if WITH_EDITOR
	// Condition on not Game World instead of is Editor World because we do not want this
	// block to run in Play In Editor sessions.
	if (GetWorld() != nullptr && !GetWorld()->IsGameWorld() && !HasAnyFlags(RF_ClassDefaultObject))
	{
		// While in the editor we don't update the wire rendering every tick and instead rely on
		// Transform Updated callbacks from the wire routing node frame parents. These callbacks
		// must be registered with each parent on start-up. We can't do that here because some of
		// the parents may not have been loaded yet. So we set up a callback to happen next tick,
		// and hope that everything has been loaded by then. Is there a better way to do this?
		//
		// A drawback of the delayed call to UpdateVisuals is that the Instanced Static Mesh
		// Components that are used for rendering the wire are marked dirty by the call to Update
		// Instance Transform, they call UObject::Modify. This means that any Level that contains
		// a Wire Component becomes dirty / unsaved on the first tick after opening. That is
		// annoying, but I'm not sure what to do about it. What happens if we remove the Update
		// Visuals call in the next-tick callback is that some Wire Routing Nodes that has a frame
		// that has an Owning Actor that is not the same as the Actor that the Wire Component is
		// part of with either not be found or have an all-zero transform. This means that parts
		// of the wire will render near the world origin instead of near the actual position of the
		// parent Scene Component until something else triggers a visual update. Also annoying.
		//
		// A workaround is to set the RF_NeedInitialization flag on the Instanced Static Mesh
		// Components before writing to them. UObject::Modify checks this flag and doesn't mark
		// the object dirty if set. Not sure if this has any undesirable side-effects. Here,
		// specifically in the next-tick callback queued from PostLoad, we want to behave

		// Make UPROPERTY? Actor callback after all Components loaded?
		UE_LOG(
			LogAGX, Warning,
			TEXT("This is not a game world and we're not the Class Default Object, setting up "
				 "parent callbacks for Wire Component %p."),
			this);
		if (MapLoadDelegateHandle.IsValid())
		{
			FEditorDelegates::MapChange.Remove(MapLoadDelegateHandle);
		}
		MapLoadDelegateHandle = FEditorDelegates::MapChange.AddWeakLambda(
			this,
			[this](uint32)
			{
				FEditorDelegates::MapChange.RemoveAll(this);
				SynchronizeParentMovedCallbacks();
				UpdateVisuals();
			});

		// In addition to setting up the callbacks.

		// If the wire routing node frame parent's owner is a Blueprint instance then any
		// modification of that instance will cause a Blueprint Reconstruction. During
		// reconstruction all the Components will be destroyed and recreated. Unfortunately, Scene
		// Component does not use Actor Component Instance Data to transfer delegate callback, such
		// as Transform Updated, from the old object to the new one, so we need to do that
		// ourselves. Fortunately, the engine provides the On Object Replaced event that we can use
		// to do the transfer.
		if (ObjectsReplacedDelegateHandle.IsValid())
		{
			FCoreUObjectDelegates::OnObjectsReinstanced.Remove(ObjectsReplacedDelegateHandle);
		}
		ObjectsReplacedDelegateHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddUObject(
			this, &UAGX_WireComponent::OnRouteNodeParentReplaced);
	}
#endif
}

#if WITH_EDITOR

void UAGX_WireComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, Radius),
		[](ThisClass* Wire) { Wire->SetRadius(Wire->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, MinSegmentLength),
		[](ThisClass* Wire) { Wire->SetMinSegmentLength(Wire->MinSegmentLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, MergeSplitProperties),
		[](ThisClass* This) { This->MergeSplitProperties.OnPostEditChangeProperty(*This); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, bCanCollide),
		[](ThisClass* Wire) { Wire->SetCanCollide(Wire->bCanCollide); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RenderMaterial),
		[](ThisClass* Wire) { Wire->SetRenderMaterial(Wire->RenderMaterial); });

	// Begin Winch.

#if 0
	Add Begin Winch Type here, and do our best to handle it.
	Alternatively, disable that setting in the Detail Panel during runtime.
#endif

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetPulledInLength(Wire->OwnedBeginWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetMotorEnabled(Wire->OwnedBeginWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetTargetSpeed(Wire->OwnedBeginWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetMotorForceRange(Wire->OwnedBeginWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetBrakeEnabled(Wire->OwnedBeginWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetBrakeForceRange(Wire->OwnedBeginWinch.BrakeForceRange); });

	/// @todo Find ways to do attach/detach during runtime from the Details Panel.

#if 0
	Add End Winch Type here, and do our best to handle it.
	Alternatively, disable that setting in the Detail Panel during runtime.
#endif

	// End Winch.
	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetPulledInLength(Wire->OwnedEndWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetMotorEnabled(Wire->OwnedEndWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetTargetSpeed(Wire->OwnedEndWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetMotorForceRange(Wire->OwnedEndWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetBrakeEnabled(Wire->OwnedEndWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetBrakeForceRange(Wire->OwnedEndWinch.BrakeForceRange); });

	/// @todo Find ways to do attach/detach from winch during runtime from the Details Panel.
}

void UAGX_WireComponent::PreEditChange(FEditPropertyChain& PropertyAboutToChange)
{
	UObject::PreEditChange(PropertyAboutToChange);

	UE_LOG(LogAGX, Warning, TEXT("PreEditChange on Wire Component %p."), this);

	// Here I would like to unsubscribe from the TransformUpdated event on the parent, but I don't
	// know how the find the index in the Route Nodes list of the routing node that is about to be
	// given a new parent. In the PostEditChangeChainProperty callback we get a
	// FPropertyChangedChainEvent instead of a FEditPropertyChain and the event knows the array
	// index because it has been created from an FPropertyHandle, which has access to FPropertyNode,
	// instead of from an FProperty. The FProperty doesn't seem to know the array index.
#if 0
	for (FEditPropertyChain::TDoubleLinkedListNode* NodeIt = PropertyAboutToChange.GetHead();
		 NodeIt != nullptr; NodeIt = NodeIt->GetNextNode())
	{
		FProperty* Property = NodeIt->GetValue();
		UE_LOG(LogAGX, Warning, TEXT("%s > "), *Property->GetName());
	}

	for (FProperty* Property : PropertyAboutToChange)
	{
		UE_LOG(LogAGX, Warning, TEXT("%s >"), *Property->GetFullName());
	}

	for (FProperty* Property : PropertyAboutToChange)
	{
		UE_LOG(LogAGX, Warning, TEXT("%s >"), *Property->GetPathName());
	}
#endif
}

void UAGX_WireComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	UE_LOG(LogAGX, Warning, TEXT("PostEditChangeChainProperty on Wire Component %p."), this);

	FEditPropertyChain::TDoubleLinkedListNode* const PropertyNode = Event.PropertyChain.GetHead();
	if (PropertyNode != nullptr)
	{
		const FName Member = PropertyNode->GetValue()->GetFName();
		if (DoesPropertyAffectVisuals(Member))
		{
			UpdateVisuals();
		}

		if (Member == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes))
		{
			// TODO Debug output, remove.
			UE_LOG(LogAGX, Warning, TEXT("Route nodes changed."));
			UE_LOG(LogAGX, Warning, TEXT("Chain:"));
			FEditPropertyChain::TDoubleLinkedListNode* It = PropertyNode;
			while (It != nullptr)
			{
				FProperty* Property = It->GetValue();
				UE_LOG(
					LogAGX, Warning, TEXT("  %s %s"), *Property->GetName(),
					*Property->GetClass()->GetName());

				It = It->GetNextNode();
			}

			// Did a node get a new parent, meaning we must set up a new parent-moved callback?
			static const TArray<const TCHAR*> Path {
				TEXT("RouteNodes"), TEXT("Frame"), TEXT("Parent")};
			if (FAGX_ObjectUtilities::HasChainPrefixPath(PropertyNode, Path))
			{
				const int32 NodeIndex = Event.GetArrayIndex("RouteNodes");
				UE_LOG(LogAGX, Warning, TEXT("  Edited index %d in RouteNodes."), NodeIndex);
				if (RouteNodes.IsValidIndex(NodeIndex))
				{
					USceneComponent* const Parent =
						RouteNodes[NodeIndex].Frame.GetParentComponent();
					if (Parent != nullptr)
					{
						FParentDelegate* ParentDelegate = DelegateHandles.Find(Parent);
						if (ParentDelegate == nullptr)
						{
							// Don't currently have a callback for this Parent, add one.
							DelegateHandles.Add(
								Parent,
								{Parent, Parent->TransformUpdated.AddUObject(
											 this, &UAGX_WireComponent::OnRouteNodeParentMoved)});

							UE_LOG(
								LogAGX, Warning, TEXT("    Adding callback to parent %p."), Parent);
						}
						else
						{
							UE_LOG(
								LogAGX, Warning, TEXT("    Parent %p already has a callback."),
								Parent);
						}
					}
				}
			}

			// We have no way of knowing what the old parent was, or if the changed node was the
			// last to have it has its parent, so the best we can do is to synchronize all node
			// parents.
			SynchronizeParentMovedCallbacks();
		}
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_WireComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		// Do not create a native AGX Dynamics object if GIsReconstructingBlueprintInstances is set.
		// That means that we're being created as part of a Blueprint Reconstruction and we will
		// soon be assigned the native that the reconstructed Wire Component had, if any.
		CreateNative();
		check(HasNative()); /// @todo Consider better error handling than check.

		MergeSplitProperties.OnBeginPlay(*this);
	}
}

void UAGX_WireComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVisuals();
}

void UAGX_WireComponent::CreateMergeSplitProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_WireComponent::CreateMergeSplitProperties was called "
				 "on Wire '%s' that does not have a Native AGX Dynamics object. Only call "
				 "this function "
				 "during play."),
			*GetName());
		return;
	}

	if (!MergeSplitProperties.HasNative())
	{
		MergeSplitProperties.CreateNative(*this);
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_WireComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_WireInstanceData>(this);
}

void UAGX_WireComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_WireComponent will inherit this one's Native Barrier, so don't wreck it.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_WireComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	UpdateVisuals();
}

void UAGX_WireComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		FName NewName = MakeUniqueObjectName(
			SpriteComponent->GetOuter(), SpriteComponent->GetClass(), TEXT("WireIcon"));
		SpriteComponent->Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
		SpriteComponent->SetSprite(
			LoadObject<UTexture2D>(nullptr, TEXT("/AGXUnreal/Editor/Icons/wire_64x64")));
		SpriteComponent->SetRelativeScale3D(FVector(2.0, 2.0, 2.0));
	}
#endif

	if (VisualCylinders == nullptr || VisualSpheres == nullptr)
		CreateVisuals();

	UpdateVisuals();
}

void UAGX_WireComponent::DestroyComponent(bool bPromoteChildren)
{
	if (VisualCylinders != nullptr)
		VisualCylinders->DestroyComponent();

	if (VisualSpheres != nullptr)
		VisualSpheres->DestroyComponent();

#if WITH_EDITOR
	if (MapLoadDelegateHandle.IsValid())
	{
		FEditorDelegates::MapChange.Remove(MapLoadDelegateHandle);
	}
	if (ObjectsReplacedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReinstanced.Remove(ObjectsReplacedDelegateHandle);
	}
#endif

	Super::DestroyComponent(bPromoteChildren);
}

namespace AGX_WireComponent_helpers
{
	class FAGXWireNotifyBuffer : public FAGXNotifyListener
	{
	public:
		virtual ~FAGXWireNotifyBuffer() = default;

		virtual void OnMessage(const FString& InMessage, ELogVerbosity::Type Verbosity) override
		{
			if (Message != "")
			{
				Message += "\n";
			}
			Message += InMessage;
		}

	public:
		FString Message;
	};

	/**
	 * Given a location in one frame of reference, return the same world location relative to
	 * another frame of reference.
	 *
	 * @param SourceTransform The frame of reference in which LocalLocation is given.
	 * @param TargetTransform The frame of reference in which we want the same world location.
	 * @param LocalLocation The location in the source frame of reference.
	 * @return The location in the target frame of reference.
	 */
	FVector MoveLocationBetweenLocalTransforms(
		const FTransform& SourceTransform, const FTransform& TargetTransform,
		const FVector& LocalLocation)
	{
		// A.GetRelativeTransform(B) produces a transformation that goes from B to A. That is, it
		// tells us where A is in relation to B. Transforming the zero vector with that transform
		// will produce the vector pointing from B to A. Transforming any other vector will produce
		// the vector pointing from B to the tip of the first vector when placed with its base at A.
		// In our case we want the vector pointing from TargetTransform so that should be our B,
		// i.e. the parameter.
		FTransform SourceToTarget = SourceTransform.GetRelativeTransform(TargetTransform);
		return SourceToTarget.TransformPosition(LocalLocation);
	}

	// TODO This code will need to change with the introduction of frame in routing nodes.
	std::tuple<FRigidBodyBarrier*, FVector> GetBodyAndLocalLocation(
		const FWireRoutingNode& RouteNode, const FTransform& WireTransform)
	{
		UAGX_RigidBodyComponent* BodyComponent = RouteNode.RigidBody.GetRigidBody();
		if (BodyComponent == nullptr)
		{
			return {nullptr, FVector::ZeroVector};
		}
		FRigidBodyBarrier* NativeBody = BodyComponent->GetOrCreateNative();
		check(NativeBody);
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
		const FVector LocalLocation =
			RouteNode.Frame.GetLocationRelativeTo(*BodyComponent, WireTransform);
#else
		const FVector LocalLocation = MoveLocationBetweenLocalTransforms(
			WireTransform, BodyComponent->GetComponentTransform(), RouteNode.Location);
#endif

		return {NativeBody, LocalLocation};
	}
}

namespace AGX_WireComponent_helpers
{
	void CreateNativeWireOwnedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		check(Wire.GetOwnedWinch(Side) != nullptr);
		FAGX_WireWinch& Winch = *Wire.GetOwnedWinch(Side);
		FAGX_WireUtilities::ComputeSimulationPlacement(Wire, Winch);
		FWireWinchBarrier* Barrier = Winch.GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create AGX Dynamics instance for wire-owned winch on Wire '%s' "
					 "in '%s'"),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWireWinchOwnedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		if (Side == EWireSide::None)
		{
			return;
		}

// Unreal Engine 5.2 require that the message passed to UE_LOG is constexpr.
// Not sure how to make the constexpr-ness of a string literal survive through a call to a lambda
// function, so falling back to a macro.
//
// Is there a better solution?
#if UE_VERSION_OLDER_THAN(5, 2, 0)

		// Message must contain four %s ordered as Wire name, Wire owner name, Winch name, Winch
		// owner name.
		auto LogError = [&Wire, Side](auto& Message)
		{
			const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);
			const FString WinchName = Reference->ComponentProperty.ToString();
			const FString ActorName = GetLabelSafe(FAGX_ObjectUtilities::GetActor(*Reference));
			UE_LOG(
				LogAGX, Error, Message, *Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName,
				*ActorName);
		};

#else

#define LogError(Message)                                                                    \
	const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);            \
	const FString WinchName = Reference->ComponentProperty.ToString();                       \
	const FString ActorName = GetLabelSafe(FAGX_ObjectUtilities::GetActor(*Reference));      \
	UE_LOG(                                                                                  \
		LogAGX, Error, Message, *Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName, \
		*ActorName);

#endif

		UAGX_WireWinchComponent* WinchComponent = Wire.GetWinchComponent(Side);
		if (WinchComponent == nullptr)
		{
			LogError(
				TEXT("Wire '%s' in '%s' did not find a Wire Winch named '%s' in '%s'. AGX Dynamics "
					 "instance will not be created."));
			return;
		}

		FWireWinchBarrier* Barrier = WinchComponent->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			LogError(
				TEXT("Wire '%s' in '%s' could not create AGX Dynamics instance for Wire Winch '%s' "
					 "in '%s'."));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWireBorrowedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		if (Side == EWireSide::None)
		{
			return;
		}

		FAGX_WireWinch* Winch = Wire.GetBorrowedWinch(Side);
		if (Winch == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Wire Winch Owner Type on '%s' in '%s' has been set to Other but no Wire "
					 "Winch as been assigned to Borrowed Winch. No AGX Dynamics winch instance "
					 "will be created."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		FWireWinchBarrier* Barrier = Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create AGX Dynamics instance for a borrowed winch on '%s' in "
					 "'%s'."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		EWireWinchOwnerType WinchType = Wire.GetWinchOwnerType(Side);
		switch (WinchType)
		{
			case EWireWinchOwnerType::Wire:
				CreateNativeWireOwnedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::WireWinch:
				CreateNativeWireWinchOwnedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::Other:
				CreateNativeWireBorrowedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::None:
				// Nothing to do here.
				break;
		}
	}
}

bool UAGX_WireComponent::SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial)
{
	UAGX_ShapeMaterial* ShapeMaterialOrig = ShapeMaterial;
	ShapeMaterial = InShapeMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeMaterial is responsible for creating an instance of none exists and do the
	// asset/instance swap.
	if (!UpdateNativeMaterial())
	{
		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

void UAGX_WireComponent::SetCanCollide(bool CanCollide)
{
	if (HasNative())
	{
		GetNative()->SetEnableCollisions(CanCollide);
	}

	bCanCollide = CanCollide;
}

bool UAGX_WireComponent::GetCanCollide() const
{
	if (HasNative())
	{
		return GetNative()->GetEnableCollisions();
	}

	return bCanCollide;
}

void UAGX_WireComponent::AddCollisionGroup(const FName& GroupName)
{
	if (GroupName.IsNone())
	{
		return;
	}

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		NativeBarrier.AddCollisionGroup(GroupName);
}

void UAGX_WireComponent::RemoveCollisionGroupIfExists(const FName& GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		NativeBarrier.RemoveCollisionGroup(GroupName);
}

void UAGX_WireComponent::SetRenderMaterial(UMaterialInterface* Material)
{
	RenderMaterial = Material;

	if (VisualCylinders != nullptr)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres != nullptr)
		VisualSpheres->SetMaterial(0, RenderMaterial);
}

void UAGX_WireComponent::CreateNative()
{
	using namespace AGX_WireComponent_helpers;

	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	const float ResolutionPerUnitLength = 1.0f / MinSegmentLength;
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);
	check(HasNative()); /// @todo Consider better error handling than 'check'.

	if (!UpdateNativeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeMaterial returned false in AGX_WireComponent. "
				 "Ensure the selected Shape Material is valid."));
	}

	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);
	NativeBarrier.SetEnableCollisions(bCanCollide);
	NativeBarrier.AddCollisionGroups(CollisionGroups);

	/// @todo Not sure if we should expose Scale Constant or not.
	// NativeBarrier.SetScaleConstant(ScaleConstant);

	const FTransform LocalToWorld = GetComponentTransform();

	// Collection of error messages related to the wire setup/configuration that the user made.
	// These are presented as a dialog box at the end of the initialization process.
	// The intention is to add the equivalent edit-time checks and display in the wire's Details
	// Panel so the user can be informed before clicking Play.
	TArray<FString> ErrorMessages;

	if (HasBeginWinch())
	{
		AGX_WireComponent_helpers::CreateNativeWinch(*this, EWireSide::Begin);
	}

	// Create AGX Dynamics simulation nodes and initialize the wire.
	for (int32 I = 0; I < RouteNodes.Num(); ++I)
	{
		FWireRoutingNode& RouteNode = RouteNodes[I];
		FWireNodeBarrier NodeBarrier;

		if (RouteNode.RigidBody.OwningActor == nullptr)
		{
			// Default route nodes to search for Rigid Bodies in the same Actor as the Wire is in,
			// unless another owner has already been specified.
			RouteNode.RigidBody.OwningActor = GetOwner();
		}

		switch (RouteNode.NodeType)
		{
			case EWireNodeType::Free:
			{
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
				const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
#else
				// TODO This code will need to change with the introduction of frame in routing
				// nodes.
				//
				// Can no longer use the Wire Component's transform, named LocalToWorld here, to go
				// from the route node's location to a location we can pass to AGX Dynamics.
				const FVector WorldLocation = LocalToWorld.TransformPosition(RouteNode.Location);
#endif
				NodeBarrier.AllocateNativeFreeNode(WorldLocation);
				break;
			}
			case EWireNodeType::Eye:
			{
				FRigidBodyBarrier* Body;
				FVector Location;
				std::tie(Body, Location) = GetBodyAndLocalLocation(RouteNode, LocalToWorld);
				if (Body == nullptr)
				{
					ErrorMessages.Add(FString::Printf(
						TEXT("Wire node at index %d has invalid body. Creating Free Node instead "
							 "of Eye Node."),
						I));
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
					const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
#else
					const FVector WorldLocation =
						LocalToWorld.TransformPosition(RouteNode.Location);
#endif
					NodeBarrier.AllocateNativeFreeNode(WorldLocation);
					break;
				}
				NodeBarrier.AllocateNativeEyeNode(*Body, Location);
				break;
			}
			case EWireNodeType::BodyFixed:
			{
				FRigidBodyBarrier* Body;
				FVector Location;
				std::tie(Body, Location) = GetBodyAndLocalLocation(RouteNode, LocalToWorld);
				if (Body == nullptr)
				{
					ErrorMessages.Add(FString::Printf(
						TEXT("Wire node at index %d has invalid body. Creating Free Node instead "
							 "for Body Fixed Node."),
						I));
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
					const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
#else
					const FVector WorldLocation =
						LocalToWorld.TransformPosition(RouteNode.Location);
#endif
					NodeBarrier.AllocateNativeFreeNode(WorldLocation);
					break;
				}
				NodeBarrier.AllocateNativeBodyFixedNode(*Body, Location);
				break;
			}
			case EWireNodeType::Other:
				UE_LOG(
					LogAGX, Warning,
					TEXT(
						"Found unexpected node type in Wire '%s', part of actor '%s', at index %d. "
						"Node ignored."),
					*GetName(), *GetLabelSafe(GetOwner()), I);
				break;
		}
		NativeBarrier.AddRouteNode(NodeBarrier);
	}

	if (HasEndWinch())
	{
		AGX_WireComponent_helpers::CreateNativeWinch(*this, EWireSide::End);
	}

	if (ErrorMessages.Num() > 0)
	{
		FString Message = FString::Printf(
			TEXT("Errors detected during initialization of wire %s in %s:\n"), *GetName(),
			*GetNameSafe(GetOwner()));
		for (const FString& Line : ErrorMessages)
		{
			Message += Line + '\n';
		}
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
	}

	{
		FAGXWireNotifyBuffer Messages;
		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
		if (Simulation == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Wire '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
					 "returned "
					 "nullptr."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return;
		}

		Simulation->Add(*this);

		if (!IsInitialized())
		{
			const FString WireName = GetName();
			const FString OwnerName = GetLabelSafe(GetOwner());
			const FString Message = FString::Printf(
				TEXT("Invalid wire configuration for '%s' in '%s':\n%s"), *WireName, *OwnerName,
				*Messages.Message);

			FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		}
	}
}

void UAGX_WireComponent::CreateVisuals()
{
	VisualCylinders =
		NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualCylinders")));
	VisualCylinders->RegisterComponent();
	VisualCylinders->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* CylinderAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualCylinder.SM_WireVisualCylinder'");
	VisualCylinders->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(CylinderAssetPath));
	VisualCylinders->SetMaterial(0, RenderMaterial);

	VisualSpheres = NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualSpheres")));
	VisualSpheres->RegisterComponent();
	VisualSpheres->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* SphereAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualSphere.SM_WireVisualSphere'");
	VisualSpheres->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(SphereAssetPath));
	VisualSpheres->SetMaterial(0, RenderMaterial);
}

bool UAGX_WireComponent::UpdateNativeMaterial()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UpdateNativeMaterial called on Wire '%s' but it does not have a "
				 "native AGX Dynamics representation."),
			*GetName());
		return false;
	}

	if (ShapeMaterial == nullptr)
	{
		if (HasNative())
		{
			GetNative()->ClearMaterial();
		}
		return true;
	}

	UWorld* World = GetWorld();
	UAGX_ShapeMaterial* MaterialInstance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(World));
	check(MaterialInstance);

	if (ShapeMaterial != MaterialInstance)
	{
		ShapeMaterial = MaterialInstance;
	}

	FShapeMaterialBarrier* MaterialBarrier =
		MaterialInstance->GetOrCreateShapeMaterialNative(World);
	check(MaterialBarrier);
	NativeBarrier.SetMaterial(*MaterialBarrier);

	return true;
}

#if WITH_EDITOR
bool UAGX_WireComponent::DoesPropertyAffectVisuals(const FName& MemberPropertyName) const
{
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, VisualCylinders))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, VisualSpheres))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, Radius))
		return true;

	return false;
}
#endif

TArray<FVector> UAGX_WireComponent::GetNodesForRendering() const
{
	TArray<FVector> NodeLocations;
	if (HasRenderNodes())
	{
		NodeLocations = GetRenderNodeLocations();
	}
	else
	{
		NodeLocations.Reserve(RouteNodes.Num());
		const FTransform& ComponentTransform = GetComponentTransform();
		for (const auto& Node : RouteNodes)
		{
#if AGX_WIRE_ROUTE_NODE_USE_FRAME
			const FVector WorldLocation = Node.Frame.GetWorldLocation(*this);
#else
			// TODO This code will need to change with the introduction of frame in routing nodes.
			//
			// Can no longer assume that Node.Location should be transformed local-to-world with
			// the Wire Component's transform.
			const FVector WorldLocation =
				ComponentTransform.TransformPositionNoScale(Node.Location);
#endif
			NodeLocations.Add(WorldLocation);
		}
	}

	return NodeLocations;
}

bool UAGX_WireComponent::ShouldRenderSelf() const
{
	return VisualCylinders != nullptr && VisualSpheres != nullptr && ShouldRender();
}

void UAGX_WireComponent::UpdateVisuals()
{
	if (!ShouldRenderSelf())
	{
		const bool hasVisualCylinders =
			VisualCylinders != nullptr && VisualCylinders->GetInstanceCount() > 0;
		const bool hasVisualSpheres =
			VisualSpheres != nullptr && VisualSpheres->GetInstanceCount() > 0;

		if (VisualCylinders || hasVisualSpheres)
			SetVisualsInstanceCount(0);

		return;
	}

	// Workaround, the RenderMaterial does not propagate properly in SetRenderMaterial() in
	// Blueprints, so we assign it here.
	if (VisualCylinders->GetMaterial(0) != RenderMaterial)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres->GetMaterial(0) != RenderMaterial)
		VisualSpheres->SetMaterial(0, RenderMaterial);

	TArray<FVector> NodeLocations = GetNodesForRendering();
	RenderSelf(NodeLocations);
}

void UAGX_WireComponent::RenderSelf(const TArray<FVector>& Points)
{
	if (Points.Num() <= 1)
		return;

	const int32 NumSegments = Points.Num() - 1;
	SetVisualsInstanceCount(NumSegments);

	VisualCylinders->UpdateComponentToWorld();
	VisualSpheres->UpdateComponentToWorld();

	const double ScaleXY = static_cast<double>(Radius) * 0.01 * 2.0;
	FTransform SphereTransform {FTransform::Identity};
	FTransform CylTransform {FTransform::Identity};
	SphereTransform.SetScale3D(FVector(ScaleXY, ScaleXY, ScaleXY));
	for (int i = 0; i < NumSegments; i++)
	{
		const FVector& StartLocation = Points[i];
		const FVector& EndLocation = Points[i + 1];
		const FVector MidPoint = (StartLocation + EndLocation) * 0.5;
		const FVector DeltaVec = EndLocation - StartLocation;
		const FRotator Rot = UKismetMathLibrary::MakeRotFromZ(DeltaVec);

		CylTransform.SetLocation(MidPoint);
		CylTransform.SetRotation(Rot.Quaternion());
		const auto Distance = (DeltaVec).Length();
		CylTransform.SetScale3D(FVector(ScaleXY, ScaleXY, Distance * 0.01));
		VisualCylinders->UpdateInstanceTransform(i, CylTransform, true, true);

		SphereTransform.SetLocation(StartLocation);
		VisualSpheres->UpdateInstanceTransform(i, SphereTransform, true, true);
	}
}

void UAGX_WireComponent::SetVisualsInstanceCount(int32 Num)
{
	Num = std::max(0, Num);

	auto SetNum = [](UInstancedStaticMeshComponent& C, int32 N)
	{
		while (C.GetInstanceCount() < N)
		{
			C.AddInstance(FTransform());
		}
		while (C.GetInstanceCount() > N)
		{
			C.RemoveInstance(C.GetInstanceCount() - 1);
		}
	};

	if (VisualCylinders != nullptr)
		SetNum(*VisualCylinders, Num);

	if (VisualSpheres != nullptr)
		SetNum(*VisualSpheres, Num);
}

#if WITH_EDITOR
void UAGX_WireComponent::SynchronizeParentMovedCallbacks()
{
	// We currently have no reliable way to detect when we should no longer be tracking
	// a parent. This is a cleanup-pass that should not be necessary but is until we figure
	// out how to reliably remove our callback from the parent's TransformUpdated event, and
	// remove an entry from the Delegate Handles list when the parent no longer exists.

	UE_LOG(LogAGX, Warning, TEXT("  Parent check-up:"));

	// Find the parents currently used by a routing node.
	TSet<USceneComponent*> ActualParents;
	for (FWireRoutingNode& RoutingNode : RouteNodes)
	{
		if (USceneComponent* Parent = RoutingNode.Frame.Parent.GetComponent<USceneComponent>())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("    Found a node with parent %p %s."), Parent,
				*Parent->GetName());
			ActualParents.Add(Parent);
			if (!Parent->TransformUpdated.IsBoundToObject(this))
			{
				if (DelegateHandles.Contains(Parent))
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("Wire Component '%s' in '%s' found a routing node parent that "
							 "don't contain our callback but we do have a delegate handle "
							 "registered for it. That should never happen."),
						*GetName(), *GetLabelSafe(GetOwner()));
					// Not sure what happened. Reset the state so that we are sure the dual-Add
					// below result in the expected state.
					DelegateHandles.Remove(Parent);
				}
				DelegateHandles.Add(
					Parent, {Parent, Parent->TransformUpdated.AddUObject(
										 this, &UAGX_WireComponent::OnRouteNodeParentMoved)});
				UE_LOG(LogAGX, Warning, TEXT("      Adding callback to parent %p."), Parent);
			}
			else
			{
				UE_LOG(LogAGX, Warning, TEXT("      Already has a callback."))
				if (!DelegateHandles.Contains(Parent))
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("Wire Component '%s' in '%s' found a routing node parent that do "
							 "contain our callback but we don't have a delegate handle for it. "
							 "That should never happen"),
						*GetName(), *GetLabelSafe(GetOwner()));

					// We have a weird state. Try to fix it.
					Parent->TransformUpdated.RemoveAll(this);
					DelegateHandles.Add(
						Parent, {Parent, Parent->TransformUpdated.AddUObject(
											 this, &UAGX_WireComponent::OnRouteNodeParentMoved)});
				}
			}

			// Sanity check.
			if (DelegateHandles.Contains(Parent) && !Parent->TransformUpdated.IsBoundToObject(this))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Wire Component '%s' in '%s' found a delegate handle for a "
						 "routing node parent that don't have a callback registered "
						 "for. That should never happen."),
					*GetName(), *GetLabelSafe(GetOwner()));
			}
			else if (
				!DelegateHandles.Contains(Parent) && Parent->TransformUpdated.IsBoundToObject(this))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Wire Component '%s' in '%s' did not find a delegate handle for a "
						 "routing node parent that do have a callback registered. That should "
						 "never happen."),
					*GetName(), *GetLabelSafe(GetOwner()));
			}
		}
	}

	// Find parents that aren't a parent to any routing node anymore, and parents that has
	// been destroyed.
	TSet<USceneComponent*> ToRemove;
	UE_LOG(LogAGX, Warning, TEXT("Tracked parents:"));
	for (TTuple<USceneComponent*, FParentDelegate>& Entry : DelegateHandles)
	{
		// Parent may have been destroyed. In this case the key-part of the entry will retain its
		// old value, i.e. it will be a dangling pointer, while the TWeakObjectPtr in the value-part
		// of the entry will have been set to null.
		//
		// DO NOT USE ParentKey as an actual USceneComponent! Only use it to erase from
		// DelegateHandles.
		USceneComponent* ParentKey = Entry.Key;
		TWeakObjectPtr ParentValue = Entry.Value.Parent;

		UE_LOG(
			LogAGX, Warning, TEXT("  %p -> %p (IsValid %d)"), ParentKey, ParentValue.Get(),
			ParentValue.IsValid());

		if (!ParentValue.IsValid())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("    Found handle to invalid parent %p. Removing"),
				ParentKey);
			ToRemove.Add(ParentKey);
		}

		if (!ActualParents.Contains(ParentKey))
		{
			UE_LOG(
				LogAGX, Warning, TEXT("    Found handle to parent %p that isn't a parent anymore."),
				ParentKey);
			ToRemove.Add(ParentKey);
		}
	}

	// Remove all parents that no routing node use.
	for (USceneComponent* NoLongerParent : ToRemove)
	{
		if (IsValid(NoLongerParent))
		{
			NoLongerParent->TransformUpdated.RemoveAll(this);
		}
		DelegateHandles.Remove(NoLongerParent);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
