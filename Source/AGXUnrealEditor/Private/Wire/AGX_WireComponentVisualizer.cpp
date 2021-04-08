#include "Wire/AGX_WireComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "SceneManagement.h"
#include "UnrealEngine.h"

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

void FAGX_WireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Component);
	if (Wire == nullptr)
	{
		return;
	}

	const float NodeHandleSize = 10.0f;
	const FTransform& LocalToWorld = Wire->GetComponentTransform();
	const TArray<FWireNode>& Nodes = Wire->Nodes;
	const int32 NumNodes = Nodes.Num();

	// The Location of the previous drawn node. Used to draw the line.
	FVector PrevLocation;

	for (int32 I = 0; I < NumNodes; ++I)
	{
		const FVector Location = LocalToWorld.TransformPosition(Nodes[I].Location);
		const FLinearColor Color =
			I == SelectedNodeIndex ? GEngine->GetSelectionOutlineColor() : FLinearColor::White;

		PDI->SetHitProxy(new HNodeProxy(Wire, I));
		PDI->DrawPoint(Location, Color, NodeHandleSize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);

		if (I > 0)
		{
			PDI->DrawLine(PrevLocation, Location, FLinearColor::White, SDPG_Foreground);
		}
		PrevLocation = Location;
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
		return false;
	}

	if (HNodeProxy* NodeProxy = HitProxyCast<HNodeProxy>(VisProxy))
	{
		// const UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(VisProxy->Component.Get());
		UE_LOG(LogAGX, Warning, TEXT("Clicked node %d."), NodeProxy->NodeIndex);
		if (NodeProxy->NodeIndex == SelectedNodeIndex)
		{
			SelectedNodeIndex = INDEX_NONE;
			SelectedWire = nullptr;
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
	if (SelectedWire == nullptr)
	{
		return false;
	}
	if (!SelectedWire->Nodes.IsValidIndex(SelectedNodeIndex))
	{
		return false;
	}

	const FTransform& LocalToWorld = SelectedWire->GetComponentTransform();
	OutLocation = LocalToWorld.TransformPosition(SelectedWire->Nodes[SelectedNodeIndex].Location);
	return true;
}

void FAGX_WireComponentVisualizer::EndEditing()
{
	SelectedNodeIndex = INDEX_NONE;
}
