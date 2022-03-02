// Copyright 2022, Algoryx Simulation AB.

#include "Wire/AGX_WireComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_RuntimeStyle.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireHitProxies.h"
#include "Wire/AGX_WireNode.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/AGX_WireWinch.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "SceneManagement.h"
#include "ScopedTransaction.h"
#include "UnrealEngine.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "AGX_WireComponentVisualizer"

class FWireVisualizerOperations
{
public:
	static bool NodeProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire, HNodeProxy& Proxy)
	{
		if (Wire.IsInitialized())
		{
			// Node selection is currently only for route nodes. All node manipulation operations
			// operate on the route nodes, but when the wire is initialized what we're seeing is
			// simulation nodes.
			Visualizer.ClearSelection();
			return false;
		}
		if (Proxy.NodeIndex == Visualizer.SelectedNodeIndex)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearSelection();
			return true;
		}
		// A new node became selected.
		SelectNode(Visualizer, Wire, Proxy.NodeIndex);
		return true;
	}

	static void SelectNode(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire, int32 NodeIndex)
	{
		Visualizer.SelectedWinch = EWireSide::None;
		Visualizer.SelectedWinchSide = EWinchSide::None;
		Visualizer.SelectedNodeIndex = NodeIndex;
		Visualizer.WirePropertyPath = FComponentPropertyPath(&Wire);
	}

	static bool WinchLocationProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		HWinchLocationProxy& Proxy)
	{
		return WinchProxyClicked(Visualizer, Wire, Proxy.Side, EWinchSide::Location);
	}

	static bool WinchDirectionProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		HWinchDirectionProxy& Proxy)
	{
		return WinchProxyClicked(Visualizer, Wire, Proxy.Side, EWinchSide::Rotation);
	}

	static bool WinchProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		EWireSide& WireSide, EWinchSide WinchSide)
	{
		if (Wire.IsInitialized())
		{
			/// @todo Figure out how, if possible, one can move/orient an AGX Dynamics Wire Winch
			/// Controller during runtime. For now we don't allow editing a Wire Winch after the
			/// simulation has started.
			Visualizer.ClearSelection();
			return false;
		}
		if (Visualizer.SelectedWinch == WireSide && Visualizer.SelectedWinchSide == WinchSide)
		{
			// Clicking a selected winch deselects it.
			Visualizer.ClearSelection();
			return true;
		}
		// A new winch became selected.
		SelectWinch(Visualizer, Wire, WireSide, WinchSide);
		return true;
	}

	static void SelectWinch(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		EWireSide WireSide, EWinchSide WinchSide)
	{
		Visualizer.SelectedWinch = WireSide;
		Visualizer.SelectedWinchSide = WinchSide;
		Visualizer.SelectedNodeIndex = INDEX_NONE;
		Visualizer.WirePropertyPath = FComponentPropertyPath(&Wire);
	}

	static void NodeProxyDragged(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		FEditorViewportClient& ViewportClient, const FVector& DeltaTranslate)
	{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		if (DeltaTranslate.IsZero() || ViewportClient.GetWidgetMode() != FWidget::WM_Translate)
#else
		if (DeltaTranslate.IsZero() || ViewportClient.GetWidgetMode() != UE::Widget::WM_Translate)
#endif
		{
			return;
		}

		/// @todo Is this Modify necessary? Compare with SplineComponentVisualizer.
		Wire.Modify();

		if (ViewportClient.IsAltPressed())
		{
			// A drag with Alt held down means that the current node should be duplicated and the
			// copy selected.

			if (!Visualizer.bIsDuplicatingNode)
			{
				// This is the start of a duplication drag. Create the duplicate and select it.
				DuplicateNode(Visualizer, Wire);
			}
			else
			{
				// This is a continuation of a previously started duplication drag. Move the
				// selected node, i.e., the copy.
				MoveNode(Visualizer, Wire, DeltaTranslate);
			}
		}
		else
		{
			// This is a regular drag, move the selected node.
			MoveNode(Visualizer, Wire, DeltaTranslate);
		}
	}

	static void DuplicateNode(FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire)
	{
		Visualizer.bIsDuplicatingNode = true;
		int32 NewNodeIndex = Visualizer.SelectedNodeIndex + 1;
		FWireRoutingNode Clone = FWireRoutingNode(Wire.RouteNodes[Visualizer.SelectedNodeIndex]);
		Wire.RouteNodes.Insert(Clone, NewNodeIndex);
		Visualizer.SelectedNodeIndex = NewNodeIndex;
		Visualizer.NotifyPropertyModified(&Wire, Visualizer.RouteNodesProperty);
	}

	static void MoveNode(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		const FVector& DeltaTranslate)
	{
		const FTransform& LocalToWorld = Wire.GetComponentTransform();
		FWireRoutingNode& SelectedNode = Wire.RouteNodes[Visualizer.SelectedNodeIndex];
		const FVector CurrentLocalLocation = SelectedNode.Location;
		const FVector CurrentWorldLocation = LocalToWorld.TransformPosition(CurrentLocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		const FVector NewLocalLocation = LocalToWorld.InverseTransformPosition(NewWorldLocation);
		SelectedNode.Location = NewLocalLocation;
		Visualizer.NotifyPropertyModified(&Wire, Visualizer.RouteNodesProperty);
	}

	static void WinchProxyDragged(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		const FVector& DeltaTranslate, const FRotator& DeltaRotate)
	{
		FAGX_WireWinch& Winch = *Wire.GetWinch(Visualizer.SelectedWinch);
		const FTransform& WinchToWorld =
			FAGX_WireUtilities::GetWinchLocalToWorld(Wire, Visualizer.SelectedWinch);
		AGX_WireVisualization_helpers::TransformWinch(
			Winch, WinchToWorld, Visualizer.SelectedWinchSide, DeltaTranslate, DeltaRotate);

		FProperty* EditedProperty = Visualizer.SelectedWinch == EWireSide::Begin
										? Visualizer.BeginWinchProperty
										: Visualizer.EndWinchProperty;
		Visualizer.NotifyPropertyModified(&Wire, EditedProperty);
	}
};

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
	UClass* Class = UAGX_WireComponent::StaticClass();
	RouteNodesProperty =
		FindFProperty<FProperty>(Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes));
	BeginWinchProperty = FindFProperty<FProperty>(
		Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch));
	EndWinchProperty =
		FindFProperty<FProperty>(Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch));

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
	constexpr uint32 NumNodeColors = (uint32) EWireNodeType::NUM_NODE_TYPES;

	TStaticArray<FLinearColor, NumNodeColors> CreateWireNodeColors()
	{
		TStaticArray<FLinearColor, (uint32) EWireNodeType::NUM_NODE_TYPES> WireNodeColors;
		for (uint32 I = 0; I < NumNodeColors; ++I)
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
		static const TStaticArray<FLinearColor, NumNodeColors> WireNodeColors =
			CreateWireNodeColors();
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
			const FLinearColor NodeColor = NodeColorFunc(I, Node.NodeType);
			const FVector Location = LocalToWorld.TransformPosition(Node.Location);

			PDI->SetHitProxy(new HNodeProxy(&Wire, I));
			PDI->DrawPoint(
				Location, NodeColor, FAGX_WireUtilities::NodeHandleSize, SDPG_Foreground);
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
		auto NodeColorFunc = [](int32 I, EWireNodeType NodeType)
		{ return WireNodeTypeToColor(NodeType); };
		DrawRouteNodes(Wire, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the route nodes in a wire that is selected.
	 */
	void DrawRouteNodes(
		const UAGX_WireComponent& Wire, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = GEngine->GetSelectionOutlineColor();
		auto NodeColorFunc = [SelectedNodeIndex](int32 I, EWireNodeType NodeType)
		{
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

			PDI->DrawPoint(Location, Color, FAGX_WireUtilities::NodeHandleSize, SDPG_Foreground);
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
	using namespace AGX_WireVisualization_helpers;

	const UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Component);
	if (Wire == nullptr)
	{
		return;
	}

	if (Wire->HasBeginWinch())
	{
		const FVector WinchLocation = DrawWinch(*Wire, EWireSide::Begin, PDI);
		if (Wire->RouteNodes.Num() > 0 && !Wire->IsInitialized())
		{
			// Do not render the implicit begin-winch-to-first-node line because the render iterator
			// does provide that line along with all the other lines. The route nodes does not.
			/// @todo For nodes attached to a body, use the body's transformation instead.
			const FTransform& LocalToWorld = Wire->GetComponentTransform();
			const FVector LocalLocation = Wire->RouteNodes[0].Location;
			const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
			PDI->DrawLine(WinchLocation, WorldLocation, FLinearColor::White, SDPG_Foreground);
		}
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

	if (Wire->HasEndWinch())
	{
		const FVector& WinchLocation = DrawWinch(*Wire, EWireSide::End, PDI);
		if (Wire->RouteNodes.Num() > 0 && !Wire->IsInitialized())
		{
			// Do not render the implicit begin-winch-to-first-node line because the render iterator
			// does provide that line along with all the other lines. The route nodes does not.
			/// @todo For nodes attached to a body, use the body's transformation instead.
			const FTransform& LocalToWorld = Wire->GetComponentTransform();
			const FVector LocalLocation = Wire->RouteNodes.Last().Location;
			const FVector WorldLocation = LocalToWorld.TransformPosition(LocalLocation);
			PDI->DrawLine(WorldLocation, WinchLocation, FLinearColor::White, SDPG_Foreground);
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
		// Don't reuse selection data between Actors. It's completely different wires.
		ClearSelection();
	}

	if (HNodeProxy* Proxy = HitProxyCast<HNodeProxy>(VisProxy))
	{
		return FWireVisualizerOperations::NodeProxyClicked(*this, *Wire, *Proxy);
	}
	if (HWinchLocationProxy* Proxy = HitProxyCast<HWinchLocationProxy>(VisProxy))
	{
		return FWireVisualizerOperations::WinchLocationProxyClicked(*this, *Wire, *Proxy);
	}
	if (HWinchDirectionProxy* Proxy = HitProxyCast<HWinchDirectionProxy>(VisProxy))
	{
		return FWireVisualizerOperations::WinchDirectionProxyClicked(*this, *Wire, *Proxy);
	}

	// Add additional Proxy types here when needed.
	// Or add a virtual function, that could work as well.

	// The give proxy isn't one of ours, return false to pass on to the next handler in line.
	return false;
}

// Called by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected node, if there is one, or on the selected winch handle, if there is one.
bool FAGX_WireComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_WireComponent* Wire = GetSelectedWire();
	if (Wire == nullptr)
	{
		return false;
	}

	if (HasValidNodeSelection())
	{
		// Convert the wire-local location to a world location.
		const FTransform& LocalToWorld = Wire->GetComponentTransform();
		/// @todo Body Fixed and Eye should be relative to the body, not the wire.
		const FVector LocalLocation = Wire->RouteNodes[SelectedNodeIndex].Location;
		OutLocation = LocalToWorld.TransformPosition(LocalLocation);
		return true;
	}
	else if (HasValidWinchSelection())
	{
		const FAGX_WireWinch* Winch = Wire->GetWinch(SelectedWinch);
		checkf(
			Winch != nullptr,
			TEXT("HasValidWinchSelection has been checked but we still didn't get a winch."));

		FAGX_WireWinchPose WinchPose = FAGX_WireUtilities::GetWireWinchPose(*Wire, SelectedWinch);
		return AGX_WireVisualization_helpers::GetWidgetLocation(
			WinchPose, SelectedWinchSide, OutLocation);
	}
	return false;
}

// Called by Unreal Editor when the transform widget is moved, rotated, or scaled.
bool FAGX_WireComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	using namespace AGX_WireVisualization_helpers;
	using namespace AGX_WireComponentVisualizer_helpers;

	UAGX_WireComponent* Wire = GetSelectedWire();
	if (Wire == nullptr)
	{
		return false;
	}
	if (Wire->HasNative())
	{
		// Currently only allow direct node editing at edit time, i.e., when not having a native.
		return false;
	}

	if (HasValidNodeSelection())
	{
		FWireVisualizerOperations::NodeProxyDragged(*this, *Wire, *ViewportClient, DeltaTranslate);
	}
	else if (HasValidWinchSelection())
	{
		FWireVisualizerOperations::WinchProxyDragged(*this, *Wire, DeltaTranslate, DeltaRotate);
	}
	else
	{
		// We got a move request but we have no valid selection so don't know what to move.
		// Something's wrong, so reset the selection state.
		ClearSelection();
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

bool FAGX_WireComponentVisualizer::IsVisualizingArchetype() const
{
	/*
	 * I have no idea what this is or why it's even a thing but for route node editing to work in
	 * the Blueprint Editor it must be here because otherwise
	 * FSCSEditorViewportClient::GetWidgetLocation never calls our GetWidgetLocation.
	 */
	UAGX_WireComponent* Wire = GetSelectedWire();
	if (Wire == nullptr)
	{
		return false;
	}
	AActor* Owner = Wire->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

// I assume this is called by Unreal Editor, but not sure when, or what we should do here.
void FAGX_WireComponentVisualizer::EndEditing()
{
	ClearSelection();
}

bool FAGX_WireComponentVisualizer::HasValidNodeSelection() const
{
	return GetSelectedWire() != nullptr &&
		   !GetSelectedWire()
				->IsInitialized() && // Node selection is currently only for route nodes.
		   GetSelectedWire()->RouteNodes.IsValidIndex(SelectedNodeIndex);
}

bool FAGX_WireComponentVisualizer::HasValidWinchSelection() const
{
	/// \note Should we allow any type of winch or only owned winches? This will also accept Wire
	/// Winch owned winches and freestanding borrowed winches. Can this be handled properly in
	/// HandleInputDelta?
	return GetSelectedWire() != nullptr && !GetSelectedWire()->IsInitialized() &&
		   SelectedWinch != EWireSide::None && SelectedWinchSide != EWinchSide::None &&
		   GetSelectedWire()->HasWinch(SelectedWinch);
}

UAGX_WireComponent* FAGX_WireComponentVisualizer::GetSelectedWire() const
{
	return Cast<UAGX_WireComponent>(WirePropertyPath.GetComponent());
}

int32 FAGX_WireComponentVisualizer::GetSelectedNodeIndex() const
{
	return SelectedNodeIndex;
}

void FAGX_WireComponentVisualizer::SetSelectedNodeIndex(int32 InIndex)
{
	SelectedNodeIndex = InIndex;
}

void FAGX_WireComponentVisualizer::ClearSelection()
{
	bIsDuplicatingNode = false;
	SelectedNodeIndex = INDEX_NONE;
	SelectedWinch = EWireSide::None;
	WirePropertyPath.Reset();
}

void FAGX_WireComponentVisualizer::OnDeleteKey()
{
	if (!HasValidNodeSelection())
	{
		ClearSelection();
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteWireNode", "Delete wire node"));

	GetSelectedWire()->Modify();
	GetSelectedWire()->RouteNodes.RemoveAt(SelectedNodeIndex);
	SelectedNodeIndex = INDEX_NONE;
	bIsDuplicatingNode = false;

	NotifyPropertyModified(GetSelectedWire(), RouteNodesProperty);

	GEditor->RedrawLevelEditingViewports(true);
}

bool FAGX_WireComponentVisualizer::CanDeleteKey() const
{
	return HasValidNodeSelection();
}

#undef LOCTEXT_NAMESPACE
