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

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

bool UAGX_WireRouteNode_FL::SetBody(
	UPARAM(ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
{
	if (Body == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Nullptr Rigid Body passed to SetBody on a Wire Route Node."));
		return false;
	}
	WireNode.RigidBody.OwningActor = Body->GetOwner();
	WireNode.RigidBody.BodyName = Body->GetFName();
	return true;
}

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

FAGX_WireWinch_BP UAGX_WireComponent::GetOwnedBeginWinch_BP()
{
	return {&OwnedBeginWinch};
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

FAGX_WireWinch_BP UAGX_WireComponent::GetBeginWinchComponentWinch_BP()
{
	return {GetBeginWinchComponentWinch()};
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
	}
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
	}
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
	}

	return nullptr;
}

FAGX_WireWinch_BP UAGX_WireComponent::GetBeginWinch_BP()
{
	return {GetBeginWinch()};
}

/*
 * End Winch.
 */

FAGX_WireWinch_BP UAGX_WireComponent::GetOwnedEndWinch_BP()
{
	return {&OwnedEndWinch};
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

FAGX_WireWinch_BP UAGX_WireComponent::GetEndWinchComponentWinch_BP()
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
	}
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
	}
	return nullptr;
}

FAGX_WireWinch_BP UAGX_WireComponent::GetEndWinch_BP()
{
	return {GetEndWinch()};
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
	}
	return nullptr;
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
				TEXT("Wire side None passed to Set Winch Type for wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return;
	}

	checkNoEntry();
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
				TEXT("Wire side None passed to Get Winch Type for wire '%s' in '%s'."), *GetName(),
				*GetNameSafe(GetOwner()));
			return EWireWinchOwnerType::None;
	}

	checkNoEntry();
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
	checkNoEntry();
	return nullptr;
}

bool UAGX_WireComponent::HasWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::None:
			return false;
		case EWireSide::Begin:
			return HasBeginWinch();
		case EWireSide::End:
			return HasEndWinch();
	}
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::None:
			return nullptr;
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
	}
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::None:
			return nullptr;
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
	}
	return nullptr;
}

FAGX_WireWinch_BP UAGX_WireComponent::GetWinch_BP(EWireSide Side)
{
	return {GetWinch(Side)};
}

EWireWinchOwnerType UAGX_WireComponent::GetWinchOwnerType(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::None:
			return EWireWinchOwnerType::None;
		case EWireSide::Begin:
			return BeginWinchType;
		case EWireSide::End:
			return EndWinchType;
	}
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
	return nullptr;
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
	if (!HasNative())
	{
		/// @todo Compute the length of the route.
		return 0.0;
	}
	return NativeBarrier.GetRestLength();
}

double UAGX_WireComponent::GetMass() const
{
	if (!HasNative())
	{
		/// @todo What is reasonable to return here? Estimate the mass from material density and
		/// rest length?
		return 0.0;
	}
	return NativeBarrier.GetMass();
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
	/// @todo Implement UAGX_WireComponent::CopyFrom.
	UE_LOG(LogAGX, Error, TEXT("UAGX_WireComponent::CopyFrom not yet implemented."));
}

bool UAGX_WireComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_WireComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_WireComponent::AssignNative(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

FWireBarrier* UAGX_WireComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		checkf(
			!GIsReconstructingBlueprintInstances,
			TEXT("This is a bad situation. Someone need this Component's native but we're in the "
				 "middle of a RerunConstructionScripts and this Component haven't been given its "
				 "Native yet. We can't create a new one since we will be given the actual Native "
				 "soon, but we also can't return the actual Native right now because it hasn't "
				 "been restored from the UActorComponentInstanceData yet."));

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

void UAGX_WireComponent::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	FAGX_UpropertyDispatcher<ThisClass>& Dispatcher = FAGX_UpropertyDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	// Begin Winch.
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

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bEmergencyBrakeEnabled),
		[](ThisClass* Wire) {
			Wire->OwnedBeginWinch.SetEmergencyBrakeEnabled(
				Wire->OwnedBeginWinch.bEmergencyBrakeEnabled);
		});

	/// @todo Find ways to do attach/detach during runtime from the Details Panel.

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

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bEmergencyBrakeEnabled),
		[](ThisClass* Wire) {
			Wire->OwnedEndWinch.SetEmergencyBrakeEnabled(
				Wire->OwnedEndWinch.bEmergencyBrakeEnabled);
		});

	/// @todo Find ways to do attach/detach during runtime from the Details Panel.
#endif
}

#if WITH_EDITOR
void UAGX_WireComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_WireComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.PropertyChain.Num() > 2)
	{
		// The cases fewer chain elements are handled by PostEditChangeProperty, which is called by
		// UObject's PostEditChangeChainProperty.
		FAGX_UpropertyDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent, this);
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that his object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
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
		UAGX_Simulation::GetFrom(this)->RemoveWire(*this);
	}
	NativeBarrier.ReleaseNative();
}

void UAGX_WireComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
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
		check(Wire.GetWinch(Side) != nullptr);
		FAGX_WireWinch& Winch = *Wire.GetWinch(Side);
		FAGX_WireUtilities::ComputeSimulationPlacement(Wire, Winch);
		FWireWinchBarrier* Barrier = Winch.GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
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

		UAGX_WireWinchComponent* WinchComponent = Wire.GetWinchComponent(Side);
		if (WinchComponent == nullptr)
		{
			const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);
			const FString WinchName = Reference->ComponentProperty.ToString();
			const FString ActorName = GetLabelSafe(Reference->OtherActor);
			UE_LOG(
				LogAGX, Error,
				TEXT("Wire '%s' in '%s' did not find a Wire Winch named '%s' in '%s'. AGX Dynamics "
					 "instance will not be created."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName, *ActorName);
			return;
		}

		FWireWinchBarrier* Barrier = WinchComponent->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);
			const FString WinchName = Reference->ComponentProperty.ToString();
			const FString ActorName = GetLabelSafe(Reference->OtherActor);
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not create AGX Dynamics instance for a winch on '%s' in '%s'. Winch is "
					 "owned by '%s' in '%s'."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName, *ActorName);
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
				LogAGX, Warning,
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
				LogAGX, Warning,
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
						"Found unexpected node type in wire '%s', part of actor '%s', at index %d. "
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
		bool Added = UAGX_Simulation::GetFrom(this)->AddWire(*this);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics rejected Wire '%s' in '%s'. See the LogAGXDynamics log channel "
					 "for details."),
				*GetName(), *GetNameSafe(GetOwner()));
		}
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
