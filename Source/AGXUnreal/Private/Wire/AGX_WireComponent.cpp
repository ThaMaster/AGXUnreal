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

void UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode)
{
	RouteNodes.Add(InNode);
}

void UAGX_WireComponent::AddNodeAtLocation(const FVector& InLocation)
{
	RouteNodes.Add(FWireRoutingNode(InLocation));
}

void UAGX_WireComponent::AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex)
{
	RouteNodes.Insert(InNode, InIndex);
}

void UAGX_WireComponent::AddNodeAtLocationAtIndex(const FVector& InLocation, int32 InIndex)
{
	RouteNodes.Insert(FWireRoutingNode(InLocation), InIndex);
}

void UAGX_WireComponent::RemoveNode(int32 InIndex)
{
	RouteNodes.RemoveAt(InIndex);
}

void UAGX_WireComponent::SetNodeLocation(int32 InIndex, const FVector& InLocation)
{
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
}

void UAGX_WireComponent::CreateNative()
{
	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);
	check(HasNative()); /// @todo Consider better error handling than 'check'.

	/// @todo Create AGX Dynamics route notes and initialize the wire.
	for (int32 I = 0; I < RouteNodes.Num(); ++I)
	{
		FWireRoutingNode& RouteNode = RouteNodes[I];
		FWireNodeBarrier NodeBarrier;
		switch (RouteNode.NodeType)
		{
			case EWireNodeType::FreeNode:
			{
				const FTransform& LocalToWorld = GetComponentTransform();
				const FVector WorldLocation = LocalToWorld.TransformPosition(RouteNode.Location);
				NodeBarrier.AllocateNativeFreeNode(WorldLocation);
				break;
			}
			case EWireNodeType::EyeNode:
			{
				UAGX_RigidBodyComponent* BodyComponent = RouteNode.RigidBody.GetRigidBody();
				check(BodyComponent);
				FRigidBodyBarrier* NativeBody = BodyComponent->GetOrCreateNative();
				check(NativeBody);
				const FTransform& Transform = BodyComponent->GetComponentTransform();
				const FVector LocalLocation =
					Transform.InverseTransformPosition(RouteNode.Location);
				NodeBarrier.AllocateNativeEyeNode(*NativeBody, LocalLocation);
				break;
			}
			case EWireNodeType::BodyFixedNode:
			{
				UAGX_RigidBodyComponent* BodyComponent = RouteNode.RigidBody.GetRigidBody();
				check(BodyComponent);
				FRigidBodyBarrier* NativeBody = BodyComponent->GetOrCreateNative();
				check(NativeBody);
				const FTransform& Transform = BodyComponent->GetComponentTransform();
				const FVector LocalLocation =
					Transform.InverseTransformPosition(RouteNode.Location);
				NodeBarrier.AllocateNativeBodyFixedNode(*NativeBody, LocalLocation);
				break;
			}
		}
		NativeBarrier.AddRouteNode(NodeBarrier);
	}

	{
		using namespace AGX_WireComponent_helpers;
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
