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

class HNodeProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HNodeProxy(const UAGX_WireComponent* InWire, int32 InNodeIndex)
		: HComponentVisProxy(InWire, HPP_Wireframe)
		, NodeIndex(InNodeIndex)
	{
	}

	int32 NodeIndex;
};

IMPLEMENT_HIT_PROXY(HNodeProxy, HComponentVisProxy);

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

	FVector DrawNode(
		const UAGX_WireComponent& Wire, int32 NodeIndex, int32 SelectedNodeIndex,
		EWireNodeType NodeType, const FVector& Location, const FVector& PrevLocation,
		FPrimitiveDrawInterface* PDI)
	{
		const float NodeHandleSize = 10.0f;
		const FLinearColor Color = NodeIndex == SelectedNodeIndex
									   ? GEngine->GetSelectionOutlineColor()
									   : WireNodeTypeToColor(NodeType);

		PDI->SetHitProxy(new HNodeProxy(&Wire, NodeIndex));
		PDI->DrawPoint(Location, Color, NodeHandleSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);

		if (NodeIndex > 0)
		{
			PDI->DrawLine(PrevLocation, Location, FLinearColor::White, SDPG_Foreground);
		}
		return Location;
	}

	void DrawRoutingNodes(
		const UAGX_WireComponent& Wire, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		const FTransform& LocalToWorld = Wire.GetComponentTransform();
		const TArray<FWireRoutingNode>& Nodes = Wire.RouteNodes;
		const int32 NumNodes = Nodes.Num();

		// The Location of the previous drawn node. Used to draw the line.
		FVector PrevLocation;

		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FVector Location = LocalToWorld.TransformPosition(Nodes[I].Location);
			PrevLocation = DrawNode(
				Wire, I, SelectedNodeIndex, Nodes[I].NodeType, Location, PrevLocation, PDI);
		}
	}

	void DrawWireNodes(
		const UAGX_WireComponent& Wire, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		int32 I = 0;
		FVector PrevLocation;

		/// \todo Investigate ranged-for.
		for (auto It = Wire.GetRenderBeginIterator(), End = Wire.GetRenderEndIterator(); It != End;
			 It.Inc())
		{
			FAGX_WireNode Node = It.Get();
			EWireNodeType NodeType = Node.GetType();
			//EWireNodeType NodeType = EWireNodeType::FreeNode; /// \@todo Node.OnGetNodeTypeLabel();
			const FVector Location = Node.GetWorldLocation();
			PrevLocation =
				DrawNode(Wire, I, SelectedNodeIndex, NodeType, Location, PrevLocation, PDI);
			I++;
		}
	}
}

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
		DrawWireNodes(*Wire, SelectedNodeIndex, PDI);
	}
	else
	{
		DrawRoutingNodes(*Wire, SelectedNodeIndex, PDI);
	}
}

bool FAGX_WireComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
#if 0
	if (!VisProxy->Component.IsValid())
	{
		return false;
	}
#endif

	const UAGX_WireComponent* Wire = Cast<const UAGX_WireComponent>(VisProxy->Component);
	if (Wire == nullptr)
	{
		// Clicked something not a wire, deselect whatever we had selected before.
		ClearSelection();
		return false;
	}

	if (HNodeProxy* NodeProxy = HitProxyCast<HNodeProxy>(VisProxy))
	{
		if (Wire->IsInitialized())
		{
			// Node selection is currently only for routing nodes. All node manipulation operations
			// operate on the routing nodes, but when the wire is initialized what we're seeing is
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
			SelectedNodeIndex = NodeProxy->NodeIndex;
			// Not sure what I'm supposed to do here. I want to store the wire so I can edit it
			// later in HandleInputDelta. Is there a way to get a non-const pointer to the wire
			// without having to const-cast here? SplineComponentVisualizer uses
			// FComponentPropertyPath. I don't know what that is.
			SelectedWire = const_cast<UAGX_WireComponent*>(Wire);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool FAGX_WireComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!HasValidSelection())
	{
		return false;
	}

	const FTransform& LocalToWorld = SelectedWire->GetComponentTransform();
	OutLocation =
		LocalToWorld.TransformPosition(SelectedWire->RouteNodes[SelectedNodeIndex].Location);
	return true;
}

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

	SelectedWire->Modify();
	TArray<FWireRoutingNode>& Nodes = SelectedWire->RouteNodes;

	if (ViewportClient->IsAltPressed())
	{
		if (ViewportClient->GetWidgetMode() != FWidget::WM_Translate)
		{
			return false;
		}

		if (!bIsDuplicatingNode)
		{
			bIsDuplicatingNode = true;
			int32 NewNodeIndex = SelectedNodeIndex + 1;
			SelectedWire->RouteNodes.Insert(
				FWireRoutingNode(Nodes[SelectedNodeIndex]), NewNodeIndex);
			SelectedNodeIndex = NewNodeIndex;
			NotifyPropertyModified(
				SelectedWire, FindFProperty<FProperty>(
								  UAGX_WireComponent::StaticClass(),
								  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));
		}
		else
		{
			const FTransform& LocalToWorld = SelectedWire->GetComponentTransform();
			FWireRoutingNode& SelectedNode = SelectedWire->RouteNodes[SelectedNodeIndex];
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
		const FTransform& LocalToWorld = SelectedWire->GetComponentTransform();
		FWireRoutingNode& SelectedNode = SelectedWire->RouteNodes[SelectedNodeIndex];
		const FVector CurrentLocalLocation = SelectedNode.Location;
		const FVector CurrentWorldLocation = LocalToWorld.TransformPosition(CurrentLocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		const FVector NewLocalLocation = LocalToWorld.InverseTransformPosition(NewWorldLocation);
		SelectedNode.Location = NewLocalLocation;
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}

bool FAGX_WireComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		bIsDuplicatingNode = false;
		// Not retuning here. We're just detecting the event, not performing an action.
	}

	if (Event == IE_Pressed)
	{
		return CommandList->ProcessCommandBindings(
			Key, FSlateApplication::Get().GetModifierKeys(), false);
	}

	return false;
}

void FAGX_WireComponentVisualizer::EndEditing()
{
	ClearSelection();
}

bool FAGX_WireComponentVisualizer::HasValidSelection() const
{
	return SelectedWire != nullptr &&
		   !SelectedWire->IsInitialized() && // Node selection is currently only for routing nodes.
		   SelectedWire->RouteNodes.IsValidIndex(SelectedNodeIndex);
}

UAGX_WireComponent* FAGX_WireComponentVisualizer::GetSelectedWire() const
{
	return SelectedWire;
}

int32 FAGX_WireComponentVisualizer::GetSelectedNodeIndex() const
{
	return SelectedNodeIndex;
}

void FAGX_WireComponentVisualizer::ClearSelection()
{
	bIsDuplicatingNode = false;
	SelectedNodeIndex = INDEX_NONE;
	SelectedWire = nullptr;
}

void FAGX_WireComponentVisualizer::OnDeleteKey()
{
	if (!HasValidSelection())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteWireNode", "Delete wire node"));

	SelectedWire->Modify();
	SelectedWire->RouteNodes.RemoveAt(SelectedNodeIndex);
	SelectedNodeIndex = INDEX_NONE;

	NotifyPropertyModified(
		SelectedWire, FindFProperty<FProperty>(
						  UAGX_WireComponent::StaticClass(),
						  GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes)));

	GEditor->RedrawLevelEditingViewports(true);
}

bool FAGX_WireComponentVisualizer::CanDeleteKey() const
{
	return HasValidSelection();
}

#undef LOCTEXT_NAMESPACE
