#include "Wire/AGX_WireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_UpropertyDispatcher.h"
#include "AGXUnrealBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireInstanceData.h"
#include "Wire/AGX_WireNode.h"
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

FAGX_WireWinch& UAGX_WireComponent::GetBeginWinch()
{
	// UFunctions cannot return a pointer to an FStruct, but it can return a reference to one.
	// Sometimes we don't have a Wire Winch to return, in which case we return a reference to this
	// static default constructed empty Wire Winch.
	static FAGX_WireWinch InvalidWinch;

	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return OwnedBeginWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponentWinch() ? *GetBeginWinchComponentWinch() : InvalidWinch;
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch != nullptr ? *BorrowedBeginWinch : InvalidWinch;
	}
	return InvalidWinch;
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetBeginWinchComponent()
{
	UActorComponent* ActorComponent = BeginWinchComponent.GetComponent(nullptr);
	if (ActorComponent == nullptr)
	{
		return nullptr;
	}
	return Cast<UAGX_WireWinchComponent>(ActorComponent);
}

bool UAGX_WireComponent::HasBeginWinchComponentWinch()
{
	return GetBeginWinchComponent() != nullptr;
}

FAGX_WireWinch* UAGX_WireComponent::GetBeginWinchComponentWinch()
{
	UAGX_WireWinchComponent* WinchComponent = GetBeginWinchComponent();
	if (WinchComponent == nullptr)
	{
		return nullptr;
	}
	return &WinchComponent->WireWinch;
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
		CreateNative();
		check(HasNative());
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
		~FAGXWireNotifyBuffer() = default;

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

void UAGX_WireComponent::CreateNative()
{
	using namespace AGX_WireComponent_helpers;

	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	/** Damping and Young's modulus for demonstration/experimentation purposes. Will be replaced
	 * with Wire Material shortly. */
	NativeBarrier.AllocateNative(
		Radius, ResolutionPerUnitLength, DampingBend, DampingStretch, YoungsModulusBend,
		YoungsModulusStretch);
	check(HasNative()); /// @todo Consider better error handling than 'check'.

	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);

	/// @todo Not sure if we should expose Scale Constant or not.
	// NativeBarrier.SetScaleConstant(ScaleConstant);

	const FTransform LocalToWorld = GetComponentTransform();

	// Collection of error messages related to the wire setup/configuration that the user made.
	// These are presented as a dialog box at the end of the initialization process.
	// The intention is to add the equivalent edit-time checks and display in the wire's Details
	// Panel so the user can be informed before clicking Play.
	TArray<FString> ErrorMessages;

	// Create Native for the begin winch.
	FAGX_WireWinch* BeginWinch = nullptr;
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			BeginWinch = &OwnedBeginWinch;
			break;
		case EWireWinchOwnerType::WireWinch:
			UActorComponent* WinchActorComponent = BeginWinchComponent.GetComponent(nullptr);
			if (WinchActorComponent == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Cannot create Begin Wire Winch: Begin Winch Component not set."));
			}
			else
			{
				UAGX_WireWinchComponent* WinchComponent =
					Cast<UAGX_WireWinchComponent>(WinchActorComponent);
				if (WinchComponent == nullptr)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("Cannot create Begin Wire Winch: Begin Winch Component set to "
							 "something not a Wire Winch Component"));
				}
				else
				{
					BeginWinch = &WinchComponent->WireWinch;
				}
				break;
			}
	}
	if (BeginWinch != nullptr)
	{
		if (BeginWinch->GetBodyAttachment() == nullptr)
		{
			// The Wire Winch does not have a Rigid Body, which means that the Wire Winch will be
			// attached to the world. Therefore, Location and Rotation should be in the global
			// coordinate system when passed to AGX Dynamics. The Unreal Engine instance of the
			// Wire Winch doesn't know about this, it only passes on whatever Location and Rotation
			// it got, so here we transform from the Wire Component's local coordinate system to the
			// global coordinate system ON THE GAME INSTANCE of the Wire Winch. This doesn't change
			// the editor instance.
			BeginWinch->Location = GetComponentTransform().TransformPosition(BeginWinch->Location);
			BeginWinch->Rotation = GetComponentRotation() + BeginWinch->Rotation;
		}
		FWireWinchBarrier* WinchBarrier = BeginWinch->GetOrCreateNative();
		if (WinchBarrier != nullptr)
		{
			NativeBarrier.AddWinch(*WinchBarrier);
		}
		else
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to allocate AGX Dynamics instance for Begin Winch for Wire Component "
					 "'%s' in Actor '%s'."),
				*GetName(), *GetLabelSafe(GetOwner()));
		}
	}

	// Create AGX Dynamics simulation nodes and initialize the wire.
	for (int32 I = 0; I < RouteNodes.Num(); ++I)
	{
		FWireRoutingNode& RouteNode = RouteNodes[I];
		FWireNodeBarrier NodeBarrier;
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

	if (ErrorMessages.Num() > 0)
	{
		FString Message = FString::Printf(TEXT("Errors detected during wire initialization:\n"));
		for (const FString& Line : ErrorMessages)
		{
			Message += Line + '\n';
		}
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(Message);
	}

	{
		FAGXWireNotifyBuffer Messages;
		UAGX_Simulation::GetFrom(this)->AddWire(*this);
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
