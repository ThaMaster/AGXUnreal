#include "Wire/AGX_WireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_UpropertyDispatcher.h"
#include "AGXUnrealBarrier.h"
#include "Materials/AGX_ShapeMaterialAsset.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Wire/AGX_WireInstanceData.h"
#include "Wire/AGX_WireNode.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "Components/BillboardComponent.h"
#include "CoreGlobals.h"
#include "Math/UnrealMathUtility.h"

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

void FWireRoutingNode::SetBody(UAGX_RigidBodyComponent* Body)
{
	if (Body == nullptr)
	{
		RigidBody.OwningActor = nullptr;
		RigidBody.BodyName = NAME_None;
		return;
	}

	RigidBody.OwningActor = Body->GetOwner();
	RigidBody.BodyName = Body->GetFName();
}

void UAGX_WireRouteNode_FL::SetBody(
	UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
{
	WireNode.SetBody(Body);
}

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif

	// Add a pair of default nodes to make initial editing easier.
	AddNodeAtLocation(FVector::ZeroVector);
	AddNodeAtLocation(FVector(100.0f, 0.0f, 0.0f));
}

void UAGX_WireComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}
	Radius = InRadius;
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

void UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Add(InNode);
}

void UAGX_WireComponent::AddNodeAtLocation(const FVector& InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Add(FWireRoutingNode(InLocation));
}

void UAGX_WireComponent::AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Insert(InNode, InIndex);
}

void UAGX_WireComponent::AddNodeAtLocationAtIndex(const FVector& InLocation, int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.Insert(FWireRoutingNode(InLocation), InIndex);
}

void UAGX_WireComponent::RemoveNode(int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.RemoveAt(InIndex);
}

void UAGX_WireComponent::SetNodeLocation(int32 InIndex, const FVector& InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes[InIndex].Location = InLocation;
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
	double Length = 0.0;
	for (int32 I = 1; I < RouteNodes.Num(); ++I)
	{
		Length += FVector::Distance(RouteNodes[I - 1].Location, RouteNodes[I].Location);
	}
	return Length;
}

double UAGX_WireComponent::GetMass() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMass();
	}
	if (PhysicalMaterial == nullptr)
	{
		/// @note Can we find the density that AGX Dynamics will use for wires that don't have an
		/// explicit material set?
		return 0.0;
	}
	const double Area = PI * Radius * Radius; // Assume circular cross-section.
	const double Length = GetRestLength();
	const double Density = PhysicalMaterial->Bulk.Density;
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

void UAGX_WireComponent::CopyFrom(const FWireBarrier& Barrier)
{
	Radius = Barrier.GetRadius();
	MinSegmentLength = 1.0f / Barrier.GetResolutionPerUnitLength();
	LinearVelocityDamping = static_cast<float>(Barrier.GetLinearVelocityDamping());

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
					 "incorrect scene configuration."));
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
	OwnedBeginWinch.BodyAttachment.OwningActor = GetTypedOuter<AActor>();
	OwnedEndWinch.BodyAttachment.OwningActor = GetTypedOuter<AActor>();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

void UAGX_WireComponent::InitPropertyDispatcher()
{
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
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

	/// @todo Find ways to do attach/detach during runtime from the Details Panel.
}

void UAGX_WireComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(Event, this);

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
	}
}

void UAGX_WireComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/// @todo Do we need to do anything here?
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
	else if (HasNative())
	{
		UAGX_Simulation::GetFrom(this)->Remove(*this);
	}
	NativeBarrier.ReleaseNative();
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
			LoadObject<UTexture2D>(nullptr, TEXT("/AGXUnreal/Editor/Icons/T_AGX_Wire.T_AGX_Wire")));
	}
#endif
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
		const FVector LocalLocation = MoveLocationBetweenLocalTransforms(
			WireTransform, BodyComponent->GetComponentTransform(), RouteNode.Location);
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

		// Message must contain four %s ordered as Wire name, Wire owner name, Winch name, Winch
		// owner name.
		auto LogError = [&Wire, Side](auto& Message)
		{
			const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);
			const FString WinchName = Reference->ComponentProperty.ToString();
			const FString ActorName = GetLabelSafe(Reference->OtherActor);
			UE_LOG(
				LogAGX, Error, Message, *Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName,
				*ActorName);
		};

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

void UAGX_WireComponent::CreateNative()
{
	using namespace AGX_WireComponent_helpers;

	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	const float ResolutionPerUnitLength = 1.0f / MinSegmentLength;
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);
	check(HasNative()); /// @todo Consider better error handling than 'check'.

	if (PhysicalMaterial)
	{
		UWorld* World = GetWorld();
		UAGX_ShapeMaterialInstance* MaterialInstance =
			static_cast<UAGX_ShapeMaterialInstance*>(PhysicalMaterial->GetOrCreateInstance(World));
		check(MaterialInstance);
		if (MaterialInstance != PhysicalMaterial && World != nullptr && World->IsGameWorld())
		{
			PhysicalMaterial = MaterialInstance;
		}
		FShapeMaterialBarrier* MaterialBarrier =
			MaterialInstance->GetOrCreateShapeMaterialNative(World);
		check(MaterialBarrier);
		NativeBarrier.SetMaterial(*MaterialBarrier);
	}

	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);

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
				const FVector WorldLocation = LocalToWorld.TransformPosition(RouteNode.Location);
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
					ErrorMessages.Add(
						FString::Printf(TEXT("Wire node at index %d has invalid body."), I));
					const FVector WorldLocation =
						LocalToWorld.TransformPosition(RouteNode.Location);
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
					ErrorMessages.Add(
						FString::Printf(TEXT("Wire node at index %d has invalid body."), I));
					const FVector WorldLocation =
						LocalToWorld.TransformPosition(RouteNode.Location);
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
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Message);
	}

	{
		FAGXWireNotifyBuffer Messages;
		UAGX_Simulation::GetFrom(this)->Add(*this);
		if (!IsInitialized())
		{
			const FString WireName = GetName();
			const FString OwnerName = GetLabelSafe(GetOwner());
			const FString Message = FString::Printf(
				TEXT("Invalid wire configuration for '%s' in '%s':\n%s"), *WireName, *OwnerName,
				*Messages.Message);
			/// @todo Consider other error reporting ways in shipping builds.
			/// Unless we actually do want to show a dialog box to an end-end-user.
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Message);
		}
	}
}

#undef LOCTEXT_NAMESPACE
