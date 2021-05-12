#include "Wire/AGX_WireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGXUnrealBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "Components/BillboardComponent.h"
#include "CoreGlobals.h"

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
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

bool UAGX_WireComponent::HasNative() const
{
	return NativeBarrier.HasNative();
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
	check(HasNative());
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

void UAGX_WireComponent::CopyFrom(const FWireBarrier& Barrier)
{
	/// @todo Implement UAGX_WireComponent::CopyFrom.
	UE_LOG(LogAGX, Error, TEXT("UAGX_WireComponent::CopyFrom not yet implemented."));
}

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

	/// @todo Update simulation node list, once we have one.
}

void UAGX_WireComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_WireComponent will inherit this one's Native Barrier, so don't wreck it.
	}
	else
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
		check(BodyComponent);
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
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);
	check(HasNative()); /// @todo Consider better error handling than 'check'.

	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);

	/// @todo Not sure if we should expose Scale Constant or not.
	// NativeBarrier.SetScaleConstant(ScaleConstant);

	const FTransform LocalToWorld = GetComponentTransform();

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
				NodeBarrier.AllocateNativeEyeNode(*Body, Location);
				break;
			}
			case EWireNodeType::BodyFixed:
			{
				FRigidBodyBarrier* Body;
				FVector Location;
				std::tie(Body, Location) = GetBodyAndLocalLocation(RouteNode, LocalToWorld);
				NodeBarrier.AllocateNativeBodyFixedNode(*Body, Location);
				break;
			}
			case EWireNodeType::Other:
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found expected node type in wire '%s', part of actor '%s', at index %d. "
						 "Node ignored."),
					*GetName(), *GetLabelSafe(GetOwner()), I);
				break;
		}
		NativeBarrier.AddRouteNode(NodeBarrier);
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
