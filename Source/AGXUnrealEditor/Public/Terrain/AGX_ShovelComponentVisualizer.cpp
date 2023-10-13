// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ShovelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelHitProxies.h"
#include "Terrain/AGX_ShovelUtilities.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "BlueprintEditorModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Selection.h"
#include "SSubobjectEditor.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_ShovelComponentVisualizer"
#define MEMBER(Name) GET_MEMBER_NAME_CHECKED(UAGX_ShovelComponent, Name)

struct FShovelVisualizerOperations
{
	static bool ShovelProxyClicked(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		HShovelHitProxy& Proxy)
	{
		UE_LOG(LogAGX, Warning, TEXT("ShovelProxyClicked: Clicked shovel %p."), &Shovel);
		if (Shovel.HasNative())
		{
			// Not allowed to modify the shovel configuration once the native has been created.
			Visualizer.ClearSelection();
			return false;
		}

		if (Proxy.Frame == Visualizer.SelectedFrame)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearSelection();
			return true;
		}

		// We really should select a frame.
		SelectFrame(Visualizer, Shovel, Proxy.Frame);
		return true;
	}

	static void SelectFrame(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		EAGX_ShovelFrame Frame)
	{
		Visualizer.SelectedFrame = Frame;
		Visualizer.ShovelPropertyPath = FComponentPropertyPath(&Shovel);
	}

	static void FrameProxyDragged(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		FEditorViewportClient& Viewportclient, const FVector& DeltaTranslate)
	{
		if (DeltaTranslate.IsZero() || Viewportclient.GetWidgetMode() != UE::Widget::WM_Translate)
		{
			return;
		}

		// todo Is this Modify necessary? Compare with SplineComponentVisualizer.
		Shovel.Modify();

		MoveFrame(Visualizer, Shovel, DeltaTranslate);
	}

	static void MoveFrame(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		const FVector& DeltaTranslate)
	{
		UE_LOG(LogAGX, Warning, TEXT("MoveFrame: Moving frame for shovel %p."), &Shovel);
		FAGX_Frame* Frame = Visualizer.GetSelectedFrame();
		EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		const FTransform& LocalToWorld = Frame->GetParentComponent()->GetComponentTransform();
		const FVector CurrentLocalLocation = Frame->LocalLocation;
		const FVector CurrentWorldLocation = LocalToWorld.TransformPosition(CurrentLocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		FVector NewLocalLocation = LocalToWorld.InverseTransformPosition(NewWorldLocation);
#if 1
		UE_LOG(LogAGX, Warning, TEXT("Truncating new local location for details panel"));
		FAGX_ShovelUtilities::TruncateForDetailsPanel(NewLocalLocation);
#endif
		for (UAGX_ShovelComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Shovel))
		{
			UE_LOG(LogAGX, Warning, TEXT("  Updating shovel instance %p."), Instance);
			FAGX_Frame* InstanceFrame = Instance->GetFrame(FrameSource);
			if (InstanceFrame != nullptr)
			{
				// todo Only overwrite if equal.
				InstanceFrame->LocalLocation = NewLocalLocation;
				Visualizer.NotifyPropertyModified(Instance, Visualizer.GetSelectedFrameProperty());
			}
		}
		Shovel.GetFrame(FrameSource)->LocalLocation = NewLocalLocation;
		Visualizer.NotifyPropertyModified(&Shovel, Visualizer.GetSelectedFrameProperty());
	}
};

FAGX_ShovelComponentVisualizer::FAGX_ShovelComponentVisualizer()
{
	UClass* Class = UAGX_ShovelComponent::StaticClass();
	TopEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(TopEdge));
	CuttingEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingEdge));
	CuttingDirectionProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingDirection));

	// Here is where we would register the commands class and create a command list, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

FAGX_ShovelComponentVisualizer::~FAGX_ShovelComponentVisualizer()
{
	// Here is where we would unregister the commands class, if we ever have the n eed for keyboard
	// shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();

	// Here is where we would register command list actions and bind callbacks, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FComponentVisualizer::DrawVisualization(Component, View, PDI);

	const UAGX_ShovelComponent* Shovel = Cast<UAGX_ShovelComponent>(Component);
	if (Component == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("DrawVisualization: For component %p that is not a shovel."),
			Component);
		ClearSelection();
		return;
	}

	// UE_LOG(LogAGX, Warning, TEXT("DrawVisualization: For shovel %p."), Shovel);

	// Draw the top edge.
	{
		const FVector BeginLocation = Shovel->TopEdge.Start.GetWorldLocation();
		const FVector EndLocation = Shovel->TopEdge.End.GetWorldLocation();
		FLinearColor Color = FLinearColor::White;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);

		PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeBegin));
		PDI->DrawPoint(BeginLocation, Color, FAGX_ShovelUtilities::HitProxySize, SDPG_Foreground);
		PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeEnd));
		PDI->DrawPoint(EndLocation, Color, FAGX_ShovelUtilities::HitProxySize, SDPG_Foreground);
		PDI->SetHitProxy(nullptr);
	}

	// Draw the cutting edge.
	{
		const FVector BeginLocation = Shovel->CuttingEdge.Start.GetWorldLocation();
		const FVector EndLocation = Shovel->CuttingEdge.End.GetWorldLocation();
		FLinearColor Color = FLinearColor::Red;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);
	}

	// Draw the cutting direction.
	{
		const FVector BeginLocation = Shovel->CuttingDirection.GetWorldLocation();
		const FRotator Rotation = Shovel->CuttingDirection.GetWorldRotation();
		const FVector Direction = Rotation.RotateVector(FVector::ForwardVector);
		const FVector EndLocation = BeginLocation + 100 * Direction;
		const FLinearColor Color = FLinearColor::Red; //.Desaturate(0.5f);
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);
	}
}

bool FAGX_ShovelComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_ShovelComponent* Shovel = Cast<const UAGX_ShovelComponent>(VisProxy->Component);
	if (Shovel == nullptr)
	{
		// Clicked something not a shovel, deselect whatever we had selected before.
		ClearSelection();
		return false;
	}

	UE_LOG(LogAGX, Warning, TEXT("VisProxyHandleClick: Clicked proxy for shovel %p."), Shovel);

	bool Bail = false;

	UAGX_ShovelComponent* Archetype = Cast<UAGX_ShovelComponent>(Shovel->GetArchetype());
	UAGX_ShovelComponent* CDO =
		Cast<UAGX_ShovelComponent>(UAGX_ShovelComponent::StaticClass()->GetDefaultObject(true));

	UE_LOG(LogAGX, Warning, TEXT("Shovel: %p"), Shovel);
	UE_LOG(LogAGX, Warning, TEXT("Archetype: %p"), Archetype);
	UE_LOG(LogAGX, Warning, TEXT("COD: %p"), CDO);

	TSharedPtr<IBlueprintEditor> ShovelBlueprintEditor =
		FKismetEditorUtilities::GetIBlueprintEditorForObject(Shovel, false);
	if (ShovelBlueprintEditor.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("VisProxyHandleClick: There is a Blueprint editor for shovel %p."), Shovel);
		TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
			ShovelBlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
		for (TSharedPtr<FSubobjectEditorTreeNode>& Selected : Selection)
		{
			const UActorComponent* SelectedComponent = Selected->GetComponentTemplate();
			UE_LOG(
				LogAGX, Warning,
				TEXT("VisProxyHandleClick:   Selected object in shovel Blueprint editor: %p"),
				Selected->GetComponentTemplate());
			if (SelectedComponent == Shovel)
			{
				UE_LOG(LogAGX, Warning, TEXT("VisProxyHandleClick:   This is the shovel."));
			}
		}
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("VisProxyHandleClick: There is no Blueprint editor for shovel %p."), Shovel);
	}

	TSharedPtr<IBlueprintEditor> ArchetypeBlueprintEditor =
		FKismetEditorUtilities::GetIBlueprintEditorForObject(Archetype, false);
	if (ArchetypeBlueprintEditor.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("VisProxyHandleClick: There is a Blueprint editor for Archetype %p."), Archetype);
		TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
			ArchetypeBlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
		for (TSharedPtr<FSubobjectEditorTreeNode>& Selected : Selection)
		{
			const UActorComponent* SelectedComponent = Selected->GetComponentTemplate();
			UE_LOG(
				LogAGX, Warning,
				TEXT("VisProxyHandleClick:   Selected object in Archetype Blueprint editor: %p"),
				Selected->GetComponentTemplate());
			if (SelectedComponent == Archetype)
			{
				UE_LOG(LogAGX, Warning, TEXT("VisProxyHandleClick:   This is the Archetype."));
			}
		}
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("VisProxyHandleClick: There is no Blueprint editor for archetype %p."), Archetype);
	}

	UBlueprint* ShovelBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*Shovel);
	UBlueprint* ArchetypeBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*Archetype);
	UE_LOG(LogAGX, Warning, TEXT("Shovel Blueprint: %p"), ShovelBlueprint);
	UE_LOG(LogAGX, Warning, TEXT("Archetype Blueprint: %p"), ArchetypeBlueprint);

	if (ShovelBlueprint != nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("VisProxyHandleClick: There is a Blueprint for Shovel %p."),
			Shovel);
		FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult Result =
			FAGX_BlueprintUtilities::GetSCSNodeFromComponent(*ShovelBlueprint, Shovel, true);
		if (Result.FoundNode != nullptr)
		{
			UAGX_ShovelComponent* Template =
				Cast<UAGX_ShovelComponent>(Result.FoundNode->ComponentTemplate);
			UE_LOG(
				LogAGX, Warning,
				TEXT("VisProxyHandleClick: The Shovel Blueprint has an SCS node for the "
					 "Shovel %p with templace component %p."),
				Shovel, Template);
			if (Result.FoundNode->IsSelected())
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("VisProxyHandleClick: The SCS node reports that it is selected."));
			}
			TSharedPtr<IBlueprintEditor> BlueprintEditor =
				FKismetEditorUtilities::GetIBlueprintEditorForObject(ShovelBlueprint, false);
			if (BlueprintEditor.IsValid())
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("VisProxyHandleClick: There is a Blueprint Editor for the shovel "
						 "Blueprint, getting its selection."));
				TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
					BlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
				for (TSharedPtr<FSubobjectEditorTreeNode>& Selected : Selection)
				{
					const UActorComponent* SelectedComponent = Selected->GetComponentTemplate();
					UE_LOG(
						LogAGX, Warning,
						TEXT("VisProxyHandleClick:   Selected object in shovel Blueprint editor: %p"),
						Selected->GetComponentTemplate());
					if (SelectedComponent == Shovel)
					{
						UE_LOG(LogAGX, Warning, TEXT("VisProxyHandleClick:   This is the shovel."));
					}
				}
			}
		}
		else
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Did not find a SCS Node for Shovel %p in the Blueprint."),
				Shovel);
		}
	}

	if (ArchetypeBlueprint != nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("VisProxyHandleClick: There is a Blueprint for Archetype %p."),
			Archetype);
		FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult Result =
			FAGX_BlueprintUtilities::GetSCSNodeFromComponent(*ArchetypeBlueprint, Archetype, true);
		if (Result.FoundNode != nullptr)
		{
			UAGX_ShovelComponent* Template =
				Cast<UAGX_ShovelComponent>(Result.FoundNode->ComponentTemplate);
			UE_LOG(
				LogAGX, Warning,
				TEXT("VisProxyHandleClick: The Archetype Blueprint has an SCS node for the "
					 "Archetype %p with templace component %p."),
				Archetype, Template);
			if (Result.FoundNode->IsSelected())
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("VisProxyHandleClick: The SCS node reports that it is selected."));
			}
			TSharedPtr<IBlueprintEditor> BlueprintEditor =
				FKismetEditorUtilities::GetIBlueprintEditorForObject(ArchetypeBlueprint, false);
			if (BlueprintEditor.IsValid())
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("VisProxyHandleClick: There is a Blueprint Editor for the Archetype "
						 "Blueprint, getting its selection."));
				TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
					BlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
				for (TSharedPtr<FSubobjectEditorTreeNode>& Selected : Selection)
				{
					const UActorComponent* SelectedComponent = Selected->GetComponentTemplate();
					UE_LOG(
						LogAGX, Warning,
						TEXT("VisProxyHandleClick:   Selected object in Archetype Blueprint editor: %p"),
						Selected->GetComponentTemplate());
					if (SelectedComponent == Archetype)
					{
						UE_LOG(LogAGX, Warning, TEXT("VisProxyHandleClick:   This is the Archetype."));
					}
				}
			}
		}
		else
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Did not find a SCS Node for Archetype %p in the Blueprint."),
				Archetype);
		}
	}

#if 0
	if (Bail)
	{
		return false;
	}
#endif

	AActor* OldOwningActor = ShovelPropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Shovel->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		// Don't reuse selection data between Actors, it's completely different shovels.
		ClearSelection();
	}

	if (HShovelHitProxy* Proxy = HitProxyCast<HShovelHitProxy>(VisProxy))
	{
		return FShovelVisualizerOperations::ShovelProxyClicked(*this, *Shovel, *Proxy);
	}

	// Add additional proxy types here when needed.

	// The clicked proxy isn't a Shovel proxy, return false to pass on to the next handler in line.
	return false;
}

// Call by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected frame, if there is one.
bool FAGX_ShovelComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
#if 0
	if (!Shovel->IsSelectedInEditor())
	{
		return false;
	}
#endif
	if (FAGX_Frame* Frame = GetSelectedFrame())
	{
		OutLocation = Frame->GetWorldLocation();
		return true;
	}

	return false;
}

bool FAGX_ShovelComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
	if (Shovel->HasNative())
	{
		// Not possible to modify the shovel configuration after the shovel has been created.
		return false;
	}

	AActor* Owner = Shovel->GetOwner();
	if (Owner != nullptr)
	{
		UWorld* ShovelWorld = Owner->GetWorld();
		UWorld* ViewportWorld = ViewportClient->GetPreviewScene()->GetWorld();
		UE_LOG(
			LogAGX, Warning,
			TEXT("HandleInputDelta: Selected shovel %p is in world %p, viewport world=%p"), Shovel,
			ShovelWorld, ViewportWorld);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning, TEXT("HandleInputDelta: Shovel %p does not have an owner."), Shovel);
	}

	if (HasValidFrameSection())
	{
		UAGX_ShovelComponent* ToModify = FAGX_ShovelUtilities::GetShovelToModify(Shovel);
		FShovelVisualizerOperations::FrameProxyDragged(
			*this, *ToModify, *ViewportClient, DeltaTranslate);
	}
	// Add additional selection types here, if we ever get new types.
	else
	{
		// We got a move request but we have no valid selection so don't know what to move.
		// Something's wrong, so reset the selection state.
		ClearSelection();
		return false;
	}

	GEditor->RedrawLevelEditingViewports();
	return true;
}

bool FAGX_ShovelComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return FComponentVisualizer::HandleInputKey(ViewportClient, Viewport, Key, Event);
}

bool FAGX_ShovelComponentVisualizer::IsVisualizingArchetype() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
	AActor* Owner = Shovel->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

void FAGX_ShovelComponentVisualizer::EndEditing()
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	UE_LOG(LogAGX, Warning, TEXT("Visualizer for %p is ending editing."), Shovel);

	FComponentVisualizer::EndEditing();
	ClearSelection();
}

bool FAGX_ShovelComponentVisualizer::HasValidFrameSection() const
{
	return GetSelectedFrame() != nullptr;
}

FAGX_Frame* FAGX_ShovelComponentVisualizer::GetSelectedFrame() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return nullptr;
	}
	switch (SelectedFrame)
	{
		case EAGX_ShovelFrame::None:
			return nullptr;
		case EAGX_ShovelFrame::CuttingDirection:
			return &Shovel->CuttingDirection;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
			return &Shovel->CuttingEdge.Start;
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return &Shovel->CuttingEdge.End;
		case EAGX_ShovelFrame::TopEdgeBegin:
			return &Shovel->TopEdge.Start;
		case EAGX_ShovelFrame::TopEdgeEnd:
			return &Shovel->TopEdge.End;
	}
}

EAGX_ShovelFrame FAGX_ShovelComponentVisualizer::GetSelectedFrameSource() const
{
	return SelectedFrame;
}

FProperty* FAGX_ShovelComponentVisualizer::GetSelectedFrameProperty() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return nullptr;
	}
	switch (SelectedFrame)
	{
		case EAGX_ShovelFrame::None:
			return nullptr;
		case EAGX_ShovelFrame::CuttingDirection:
			return this->CuttingDirectionProperty;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return CuttingEdgeProperty;
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return TopEdgeProperty;
	}
}

void FAGX_ShovelComponentVisualizer::ClearSelection()
{
	SelectedFrame = EAGX_ShovelFrame::None;
	ShovelPropertyPath.Reset();
}

UAGX_ShovelComponent* FAGX_ShovelComponentVisualizer::GetSelectedShovel() const
{
	return Cast<UAGX_ShovelComponent>(ShovelPropertyPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
#undef MEMBER
