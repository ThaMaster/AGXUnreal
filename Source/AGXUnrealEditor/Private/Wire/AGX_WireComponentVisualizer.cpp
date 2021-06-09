#include "Wire/AGX_WireComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RuntimeStyle.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireNode.h"

// Unreal Engine includes.
#include "Editor.h"
#include "EditorViewportClient.h"
#include "SceneManagement.h"
#include "ScopedTransaction.h"
#include "UnrealEngine.h"

#define LOCTEXT_NAMESPACE "AGX_WireComponentVisualizer"

/**
 * Data associated with clickable node visualization elements.
 */
class HNodeProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HNodeProxy(const UAGX_WireComponent* InWire, int32 InNodeIndex)
		: HComponentVisProxy(InWire, HPP_Wireframe)
		, NodeIndex(InNodeIndex)
	{
	}

	// The index of the node that the visualization that this HNodeProxy is bound to represents.
	int32 NodeIndex;
};

IMPLEMENT_HIT_PROXY(HNodeProxy, HComponentVisProxy);

/**
 * A collection of commands that can be triggered through the Wire Component Visualizer.
 */
class FAGX_WireComponentVisualizerCommands : public TCommands<FAGX_WireComponentVisualizerCommands>
{
public:
	FAGX_WireComponentVisualizerCommands()
		: TCommands<FAGX_WireComponentVisualizerCommands>(
			  "AGX_WireComponentVisualizer",
			  LOCTEXT("AGX_WireComponentVisualizer", "AGX Wire Component Visualizer"), NAME_None,
			  FAGX_RuntimeStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(
			DeleteKey, "Delete wire node.", "Delete the currently selected wire node",
			EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	}

	TSharedPtr<FUICommandInfo> DeleteKey;
};

FAGX_WireComponentVisualizer::FAGX_WireComponentVisualizer()
{
	FAGX_WireComponentVisualizerCommands::Register();
	CommandList = MakeShareable(new FUICommandList());
}

FAGX_WireComponentVisualizer::~FAGX_WireComponentVisualizer()
{
	FAGX_WireComponentVisualizerCommands::Unregister();
}

void FAGX_WireComponentVisualizer::OnRegister()
{
	const auto& Commands = FAGX_WireComponentVisualizerCommands::Get();

	CommandList->MapAction(
		Commands.DeleteKey,
		FExecuteAction::CreateSP(this, &FAGX_WireComponentVisualizer::OnDeleteKey),
		FCanExecuteAction::CreateSP(this, &FAGX_WireComponentVisualizer::CanDeleteKey));
}

namespace AGX_WireComponentVisualizer_helpers
{
	constexpr uint32 NUM_NODE_COLORS = (uint32) EWireNodeType::NUM_NODE_TYPES;

	TStaticArray<FLinearColor, NUM_NODE_COLORS> CreateWireNodeColors()
	{
		TStaticArray<FLinearColor, (uint32) EWireNodeType::NUM_NODE_TYPES> WireNodeColors;
		for (uint32 I = 0; I < NUM_NODE_COLORS; ++I)
		{
			// Fallback color for any node type not assigned below.
			WireNodeColors[I] = FLinearColor::Gray;
		}
		WireNodeColors[(int) EWireNodeType::Free] = FLinearColor::Red;
		WireNodeColors[(int) EWireNodeType::Eye] = FLinearColor::Green;
		WireNodeColors[(int) EWireNodeType::BodyFixed] = FLinearColor::Blue;
		WireNodeColors[(int) EWireNodeType::Other] = FLinearColor::White;
		return WireNodeColors;
	}

	FLinearColor WireNodeTypeToColor(EWireNodeType Type)
	{
		static TStaticArray<FLinearColor, NUM_NODE_COLORS> WireNodeColors = CreateWireNodeColors();
		const uint32 I = static_cast<uint32>(Type);
		return WireNodeColors[I];
	}

	/**
	 * Draw the route nodes in a wire, with hit proxies.
	 */
	template <typename FNodeColorFunc>
	void DrawRouteNodes(
		const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI, const FLinearColor& LineColor,
		FNodeColorFunc NodeColorFunc)
	{
		const FTransform& LocalToWorld = Wire.GetComponentTransform();
		const TArray<FWireRoutingNode>& Nodes = Wire.RouteNodes;
		const int32 NumNodes = Nodes.Num();

		FVector PrevLocation;

		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FWireRoutingNode& Node = Nodes[I];
			const float NodeHandleSize = 10.0f;
			const FLinearColor NodeColor = NodeColorFunc(I, Node.NodeType);
			const FVector Location = LocalToWorld.TransformPosition(Node.Location);

			PDI->SetHitProxy(new HNodeProxy(&Wire, I));
			PDI->DrawPoint(Location, NodeColor, NodeHandleSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);

			if (I > 0)
			{
				PDI->DrawLine(PrevLocation, Location, LineColor, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}

	/**
	 * Draw the route nodes in a wire that is not selected.
	 */
	void DrawRouteNodes(const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = FLinearColor::White;
		auto NodeColorFunc = [](int32 I, EWireNodeType NodeType) {
			return WireNodeTypeToColor(NodeType);
		};
		DrawRouteNodes(Wire, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the route nodes in a wire that is selected.
	 */
	void DrawRouteNodes(
		const UAGX_WireComponent& Wire, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = GEngine->GetSelectionOutlineColor();
		auto NodeColorFunc = [SelectedNodeIndex](int32 I, EWireNodeType NodeType) {
			return I == SelectedNodeIndex ? GEditor->GetSelectionOutlineColor()
										  : WireNodeTypeToColor(NodeType);
		};
		DrawRouteNodes(Wire, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the simulation nodes of the given Wire, including lines between them. Hit proxies
	 * are not created when drawing simulation nodes.
	 */
	void DrawSimulationNodes(const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI)
	{
		int32 I = 0;
		TOptional<FVector> PrevLocation;

		for (auto It = Wire.GetRenderBeginIterator(), End = Wire.GetRenderEndIterator(); It != End;
			 It.Inc())
		{
			const FAGX_WireNode Node = It.Get();
			const EWireNodeType NodeType = Node.GetType();
			const FLinearColor Color = WireNodeTypeToColor(NodeType);
			const FVector Location = Node.GetWorldLocation();

			const float NodeHandleSize = 10.0f;
			PDI->DrawPoint(Location, Color, NodeHandleSize, SDPG_Foreground);
			if (PrevLocation.IsSet())
			{
				PDI->DrawLine(*PrevLocation, Location, FLinearColor::White, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}
}

// Called by Unreal Editor when it's time to draw the visualization.
void FAGX_WireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_WireComponentVisualizer_helpers;

	const UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Component);
	if (Wire == nullptr)
	{
		return;
	}

	if (Wire->IsInitialized())
	{
		DrawSimulationNodes(*Wire, PDI);
	}
	else
	{
		if (Wire == GetSelectedWire())
		{
			DrawRouteNodes(*Wire, SelectedNodeIndex, PDI);
		}
		else
		{
			DrawRouteNodes(*Wire, PDI);
		}
	}
}

// Called by Unreal Editor when an element with a hit proxy of the visualization is clicked.
bool FAGX_WireComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_WireComponent* Wire = Cast<const UAGX_WireComponent>(VisProxy->Component);
	if (Wire == nullptr)
	{
		// Clicked something not a wire, deselect whatever we had selected before.
		ClearSelection();
		return false;
	}

	AActor* OldOwningActor = WirePropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Wire->GetOwner();

	if (NewOwningActor != OldOwningActor)
	{
		UE_LOG(LogAGX, Warning, TEXT("WireComponentVisualizer: Owning Actor changed."));
		ClearSelection();
	}

	if (HNodeProxy* NodeProxy = HitProxyCast<HNodeProxy>(VisProxy))
	{
		if (Wire->IsInitialized())
		{
			// Node selection is currently only for route nodes. All node manipulation operations
			// operate on the route nodes, but when the wire is initialized what we're seeing is
			// simulation nodes.
			ClearSelection();
			return false;
		}

		if (NodeProxy->NodeIndex == SelectedNodeIndex)
		{
			// Clicking a selected node deselects it.
			ClearSelection();
		}
		else
		{
			// A new node became selected.
			SelectedNodeIndex = NodeProxy->NodeIndex;
			WirePropertyPath = FComponentPropertyPath(Wire);
		}
		return true;
	}
	// Add additional Proxy types here, when needed.
	// Or add a virtual function, that could work as well.
	else
	{
		return false;
	}
}

// Called by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected node, if there is one.
bool FAGX_WireComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!HasValidSelection())
	{
		return false;
	}

	// Convert the wire-local location to a world location.
	const FTransform& LocalToWorld = GetSelectedWire()->GetComponentTransform();
	const FVector NodeLocation = GetSelectedWire()->RouteNodes[SelectedNodeIndex].Location;
	OutLocation = LocalToWorld.TransformPosition(NodeLocation);
	return true;
}

// Called by Unreal Editor when the transform widget is moved, rotated, or scaled.
bool FAGX_WireComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	if (!HasValidSelection())
	{
		ClearSelection();
		return false;
	}

	if (DeltaTranslate.IsZero())
	{
		return true;
	}

	/// @todo Is this Modify necessary? Compare with SplineComponentVisualizer.
	GetSelectedWire()->Modify();
	TArray<FWireRoutingNode>& Nodes = GetSelectedWire()->RouteNodes;

	if (ViewportClient->IsAltPressed())
	{
		if (ViewportClient->GetWidgetMode() != FWidget::WM_Translate)
		{
			return false;
		}

		// A drag with Alt held down means that the current node should be duplicated and the copy
		// selected.

		if (!bIsDuplicatingNode)
		{
			// This is the start of a duplication drag. Create the duplicate and select it.
			bIsDuplicatingNode = true;
			int32 NewNodeIndex = SelectedNodeIndex + 1;
			GetSelectedWire()->RouteNodes.Insert(
				FWireRoutingNode(Nodes[SelectedNodeIndex]), NewNodeIndex);
			SelectedNodeIndex = NewNodeIndex;
			NotifyPropertyModified(
				GetSelectedWire(), FindFProperty<FProperty>(
								  UAGX_WireComponent::StaticClass(),
								  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));
		}
		else
		{
			// This is a continuation of a previously started duplication drag. Move the selected
			// node, i.e., the copy.
			const FTransform& LocalToWorld = GetSelectedWire()->GetComponentTransform();
			FWireRoutingNode& SelectedNode = GetSelectedWire()->RouteNodes[SelectedNodeIndex];
			const FVector CurrentLocalLocation = SelectedNode.Location;
			const FVector CurrentWorldLocation =
				LocalToWorld.TransformPosition(CurrentLocalLocation);
			const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
			const FVector NewLocalLocation =
				LocalToWorld.InverseTransformPosition(NewWorldLocation);
			SelectedNode.Location = NewLocalLocation;
		}
	}
	else
	{
		// This is a regular drag, move the selected node.
		const FTransform& LocalToWorld = GetSelectedWire()->GetComponentTransform();
		FWireRoutingNode& SelectedNode = GetSelectedWire()->RouteNodes[SelectedNodeIndex];
		const FVector CurrentLocalLocation = SelectedNode.Location;
		const FVector CurrentWorldLocation = LocalToWorld.TransformPosition(CurrentLocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		const FVector NewLocalLocation = LocalToWorld.InverseTransformPosition(NewWorldLocation);
		SelectedNode.Location = NewLocalLocation;
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}

// Called by Unreal Editor when a key is pressed or released while this Visualizer is active.
bool FAGX_WireComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		bIsDuplicatingNode = false;
		// Not returning here. We're just detecting the event, not performing an action.
	}

	// Pass press events on to our Command List.
	if (Event == IE_Pressed)
	{
		return CommandList->ProcessCommandBindings(
			Key, FSlateApplication::Get().GetModifierKeys(), false);
	}

	return false;
}

// I assume this is called by Unreal Editor, but not sure when, or what we should do here.
void FAGX_WireComponentVisualizer::EndEditing()
{
	ClearSelection();
}

bool FAGX_WireComponentVisualizer::HasValidSelection() const
{
	return GetSelectedWire() != nullptr &&
		   !GetSelectedWire()->IsInitialized() && // Node selection is currently only for route nodes.
		   GetSelectedWire()->RouteNodes.IsValidIndex(SelectedNodeIndex);
}

UAGX_WireComponent* FAGX_WireComponentVisualizer::GetSelectedWire() const
{
	return Cast<UAGX_WireComponent>(WirePropertyPath.GetComponent());
}

int32 FAGX_WireComponentVisualizer::GetSelectedNodeIndex() const
{
	return SelectedNodeIndex;
}

void FAGX_WireComponentVisualizer::ClearSelection()
{
	bIsDuplicatingNode = false;
	SelectedNodeIndex = INDEX_NONE;
	WirePropertyPath.Reset();
}

void FAGX_WireComponentVisualizer::OnDeleteKey()
{
	if (!HasValidSelection())
	{
		ClearSelection();
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteWireNode", "Delete wire node"));

	GetSelectedWire()->Modify();
	GetSelectedWire()->RouteNodes.RemoveAt(SelectedNodeIndex);
	SelectedNodeIndex = INDEX_NONE;
	bIsDuplicatingNode = false;

	NotifyPropertyModified(
		GetSelectedWire(), FindFProperty<FProperty>(
						  UAGX_WireComponent::StaticClass(),
						  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));

	GEditor->RedrawLevelEditingViewports(true);
}

bool FAGX_WireComponentVisualizer::CanDeleteKey() const
{
	return HasValidSelection();
}

#undef LOCTEXT_NAMESPACE
