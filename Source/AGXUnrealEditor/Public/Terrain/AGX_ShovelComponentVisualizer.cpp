// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ShovelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
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

/**
 * A collection of helper functions called by the Shovel Visualizer.
 *
 * Made as a separate struct only to reduce the amount of code and changes in the header file.
 */
struct FShovelVisualizerOperations
{
	static bool ShovelProxyClicked(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		HShovelHitProxy& Proxy, FEditorViewportClient& InViewportClient)
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

		// All checks passed, we really should select a frame.
		Visualizer.SelectedFrame = Proxy.Frame;
		Visualizer.ShovelPropertyPath = FComponentPropertyPath(&Shovel);
		switch (Visualizer.GetSelectedFrameSource())
		{
			// Some frames (begin and end for an edge) only support translation while some
			// (directions) is mainly controlled through rotation. This code tries to switch to
			// the appropriate transform gizmo mode but fails (on Unreal Engine 5.0) while in the
			// level editor because the ModesTool is currently in tracking mode. Leaving the code
			// anyway, maybe it starts working in the future, and since it does work in the
			// Blueprint editor viewport.
			case EAGX_ShovelFrame::None:
				break;
			case EAGX_ShovelFrame::CuttingDirection:
				InViewportClient.SetWidgetMode(UE::Widget::WM_Rotate);
				break;
			case EAGX_ShovelFrame::CuttingEdgeBegin:
			case EAGX_ShovelFrame::CuttingEdgeEnd:
			case EAGX_ShovelFrame::TopEdgeBegin:
			case EAGX_ShovelFrame::TopEdgeEnd:
				InViewportClient.SetWidgetMode(UE::Widget::WM_Translate);
				break;
		}
		return true;
	}

	static void FrameProxyDragged(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		FEditorViewportClient& ViewportClient, const FVector& DeltaTranslate)
	{
		if (DeltaTranslate.IsZero() || ViewportClient.GetWidgetMode() != UE::Widget::WM_Translate)
		{
			return;
		}

		UE_LOG(LogAGX, Warning, TEXT("MoveFrame: Moving frame for shovel %p."), &Shovel);
		FAGX_Frame* Frame = Visualizer.GetSelectedFrame();
		EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		// Consider transforming DeltaTranslate to the  local coordinate system and doing the
		// add there instead of the other way around. Fewer transformations.
		const FTransform& LocalToWorld = Frame->GetParentComponent()->GetComponentTransform();
		const FVector CurrentLocalLocation = Frame->LocalLocation;
		const FVector CurrentWorldLocation = LocalToWorld.TransformPosition(CurrentLocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		FVector NewLocalLocation = LocalToWorld.InverseTransformPosition(NewWorldLocation);
		UE_LOG(LogAGX, Warning, TEXT("Truncating new local location for details panel"));
		FAGX_ShovelUtilities::TruncateForDetailsPanel(NewLocalLocation);
		Shovel.Modify();
		Shovel.GetFrame(FrameSource)->LocalLocation = NewLocalLocation;
/*
  We would like to use FComponentVisualizer::NotifyPropertyModified, but that doesn't work
  because it looks at the entire Edge or Direction property, not just the FAGX_Frame member
  we are modifying. This means that is also compares the FAGX_ComponentReference when
  checking if an archetype instance is to be updated. These may contain different bytes even
  when they are conceptually the same, i.e. when Parent.OwningActor points to the shovel's
  GetOwner(). I don't see a way to have ComponentVisualizer::NotifyPropertyModified look
  into a nested struct property and compare a specific member within that struct, I don't
  think we can pass the FProperty for FAGX_Frame::LocalLocation, at least not without
  finding a way to express the entire path from the Shovel Component to the leaf Property.
*/
#if 0
		Visualizer.NotifyPropertyModified(&Shovel, Visualizer.GetSelectedFrameProperty());
#endif

		// Instead of calling Visualizer.NotifyPropertyModified, call our shadow implementation.
		NotifyFrameModified(Visualizer, Shovel);
	}

	/**
	 * Apply the change made on the Shovel to all instances of that Shovel. If the Shovel is part
	 * of a preview, i.e. lives in the world shown in a Blueprint editor viewport, then the change
	 * is promoted to be applied on the Blueprint's CSC node's template Component and its instances
	 * instead.
	 *
	 * This does conceptually the same thing as FComponentVisualizer::NotifyPropertyModified, with
	 * the difference being that this implementation is specialized for FAGX_Frame. We cannot use
	 * FComponentVisualizer::NotifyPropertyModified because I have not been able to figure out how
	 * to use that with nested properties, i.e. when a struct member is modified, and we cannot call
	 * FComponentVisualizer::NotifyPropertyModified for the entire Shovel property because the frame
	 * contains a parent Component reference which may be conceptually equal but bit-wise non-equal,
	 * for example in the common case of a local reference, i.e. the frame has a parent that is in
	 * the same Actor.
	 *
	 * If this part of the code breaks then compare this implementation with that of
	 * FComponentVisualizer::NotifyPropertyModified, perhaps Epic Games changed something and we
	 * need to do the same change here.
	 */
	static void NotifyFrameModified(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("NotifyFrameModified for shovel %p, %s."), &Shovel,
			*Shovel.GetName());

		const EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		FProperty* FrameProperty = Visualizer.GetSelectedFrameProperty();

		auto RerunConstructionScript = [](AActor& Actor)
		{
			// todo Should the parameter be true or false?
			//      Depends on if this was the last input delta for the transform gizmo drag or not.
			//      Passing false for now because that is what calling
			//      FComponentVisualizer::NotifyFrameModified does when called with default
			//      arguments, which is what FSplineComponentVisualizer does as of Unreal
			//      Engine 5.1.
			Actor.PostEditMove(false);
		};

		{
			FPropertyChangedEvent Event(FrameProperty);
			Shovel.PostEditChangeProperty(Event);
		}

		AActor* Owner = Shovel.GetOwner();
		if (Owner == nullptr)
		{
			// Is there any case where a Component does not have an owner but does have instances?
			// That is, an archetype Component that we want to modify that does not have an Owner.
			// The FComponentVisualizer::NotifyFrameModified code does not handle this case, so we
			// early-out as well.
			return;
		}
		if (!FActorEditorUtils::IsAPreviewOrInactiveActor(Owner))
		{
			// If the owner is not a preview Actor then we don't need to do all the change
			// propagation done below. Just rerun the construction script for the Actor that was
			// directly modified.
			//
			// Similar question as with a nullptr Owner, is there no case in which a non-preview
			// Actor can contain a Component that is an archetype, i.e. has instances? The
			// FComponentVisualizer::NotifyFrameModified code does not handle that case, so we
			// early-out as well.
			RerunConstructionScript(*Owner);
			return;
		}

		UE_LOG(LogAGX, Warning, TEXT("  Is in a Blueprint."));

		// We now know that we have a Component that is part of a preview Actor, i.e. one that
		// lives in e.g. the Blueprint editor viewport.

		// The component belongs to the preview actor in the BP editor, so we need to propagate the
		// property change to the archetype. Before this, we exploit the fact that the archetype and
		// the preview actor have the old and new value of the property, respectively. So we can go
		// through all archetype instances, and if they hold the (old) archetype value, update it to
		// the new value.

		// Get archetype, which should be a Blueprint CSC node's template Component.
		UAGX_ShovelComponent* Archetype = Cast<UAGX_ShovelComponent>(Shovel.GetArchetype());
		UE_LOG(LogAGX, Warning, TEXT("  Archetype:   %p, %s."), Archetype, *Archetype->GetName());
		UE_LOG(
			LogAGX, Warning, TEXT("  Default obj: %p, %s."),
			UAGX_ShovelComponent::StaticClass()->GetDefaultObject(),
			*UAGX_ShovelComponent::StaticClass()->GetDefaultObject()->GetName());
		AGX_CHECK(Archetype != nullptr);
		if (Archetype == nullptr)
		{
			return;
		}
		if (!IsValid(Archetype))
		{
			// Not sure if the archetype can become invalid without the preview instance doing so
			// first, but bail just in case.
			return;
		}
		AGX_CHECK(Archetype != UAGX_ShovelComponent::StaticClass()->GetDefaultObject());
		if (Archetype == UAGX_ShovelComponent::StaticClass()->GetDefaultObject())
		{
			// The preview instance of a Blueprint template Component always (I hope.) has the
			// template Component as its archetype, and the template Component is never the Class
			// Default Object for the type. If we ever try to modify the Class Default Object
			// here then something has gone wrong and we should bail.
			return;
		}

		// Get all archetype instances, which should include the preview Shovel.
		TArray<UObject*> ArchetypeInstances;
		Archetype->GetArchetypeInstances(ArchetypeInstances);
		AGX_CHECK(ArchetypeInstances.Contains(&Shovel));

		// This is the old value for Local Location. Only instances that has this exact value
		// should be updated.
		const FVector OriginalValue = Archetype->GetFrame(FrameSource)->LocalLocation;

		// Among the archetype instances, find the ones that have the old value for Local Location.
		// These are the Shovels that should be updated.
		TArray<UAGX_ShovelComponent*> InstancesToUpdate;
		InstancesToUpdate.Reserve(ArchetypeInstances.Num());
		for (UObject* Instance : ArchetypeInstances)
		{
			UAGX_ShovelComponent* InstanceShovel = Cast<UAGX_ShovelComponent>(Instance);
			AGX_CHECK(IsValid(InstanceShovel));
			if (InstanceShovel == &Shovel || !IsValid(InstanceShovel))
			{
				// This is the preview Shovel that the original write was done on, do not
				// propagate to this one. Or either a nullptr instance or an instance that wasn't
				// a shovel at all, or an instance that has been marked pending kill or garbage
				// collected.
				continue;
			}

			const FVector CurrentValue = InstanceShovel->GetFrame(FrameSource)->LocalLocation;
			if (CurrentValue == OriginalValue)
			{
				InstancesToUpdate.Add(InstanceShovel);
			}
		}

		// Value that should be propagated.
		const FVector NewValue = Shovel.GetFrame(FrameSource)->LocalLocation;

		// Propagate the new value to the archetype.
		{
			Archetype->SetFlags(RF_Transactional);
			Archetype->Modify();
			AActor* ArchetypeOwner = Archetype->GetOwner();
			if (ArchetypeOwner)
			{
				ArchetypeOwner->Modify();
			}
			Archetype->GetFrame(FrameSource)->LocalLocation = NewValue;
			FPropertyChangedEvent Event(FrameProperty);
			Archetype->PostEditChangeProperty(Event);
			// todo Why not call RerunConstructionScript for ArchetypeOwner?
			// FComponentVisualizer::NotifyPropertyModified doesn't, but why not? Should we?
		}

		// Propagate the new value to the archetype instances.
		for (UAGX_ShovelComponent* InstanceToUpdate : InstancesToUpdate)
		{
			InstanceToUpdate->SetFlags(RF_Transactional);
			InstanceToUpdate->Modify();
			AActor* OwnerToUpdate = InstanceToUpdate->GetOwner();
			if (OwnerToUpdate != nullptr)
			{
				OwnerToUpdate->Modify();
			}
			InstanceToUpdate->GetFrame(FrameSource)->LocalLocation = NewValue;
			FPropertyChangedEvent Event(FrameProperty);
			InstanceToUpdate->PostEditChangeProperty(Event);
			if (OwnerToUpdate != nullptr)
			{
				RerunConstructionScript(*OwnerToUpdate);
			}
		}

		RerunConstructionScript(*Owner);
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

	UE_LOG(LogAGX, Warning, TEXT("Shovel: %p: %s"), Shovel, *Shovel->GetName());
	UE_LOG(
		LogAGX, Warning, TEXT("Archetype: %p: %s"), Archetype,
		(Archetype != nullptr ? *Archetype->GetName() : TEXT("")));
	UE_LOG(
		LogAGX, Warning, TEXT("COD: %p: %s. Parent=%p: %s. Owner=%p: %s, preview=%d"), CDO,
		*CDO->GetName(), CDO->GetArchetype(),
		(CDO->GetArchetype() != nullptr ? *CDO->GetArchetype()->GetName() : TEXT("")),
		CDO->GetOwner(), (CDO->GetOwner() != nullptr ? *CDO->GetOwner()->GetName() : TEXT("")),
		(CDO->GetOwner() != nullptr ? FActorEditorUtils::IsAPreviewOrInactiveActor(CDO->GetOwner())
									: 0));

	{
		UE_LOG(LogAGX, Warning, TEXT("All instances of the CDO:"));
		TArray<UAGX_ShovelComponent*> AllInstances =
			FAGX_ObjectUtilities::GetArchetypeInstances(*CDO);
		for (UAGX_ShovelComponent* Instance : AllInstances)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  %p: %s. Parent=%p. Owner=%p: %s, preview=%d"), Instance,
				*Instance->GetName(), Instance->GetArchetype(), Instance->GetOwner(),
				(Instance->GetOwner() != nullptr ? *Instance->GetOwner()->GetName() : TEXT("")),
				(Instance->GetOwner() != nullptr
					 ? FActorEditorUtils::IsAPreviewOrInactiveActor(Instance->GetOwner())
					 : 0));
		}
	}

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
						TEXT("VisProxyHandleClick:   Selected object in shovel Blueprint editor: "
							 "%p"),
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
						TEXT("VisProxyHandleClick:   Selected object in Archetype Blueprint "
							 "editor: %p"),
						Selected->GetComponentTemplate());
					if (SelectedComponent == Archetype)
					{
						UE_LOG(
							LogAGX, Warning, TEXT("VisProxyHandleClick:   This is the Archetype."));
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
		return FShovelVisualizerOperations::ShovelProxyClicked(
			*this, *Shovel, *Proxy, *InViewportClient);
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
		UWorld* ViewportWorld =
			(ViewportClient != nullptr && ViewportClient->GetPreviewScene() != nullptr)
				? ViewportClient->GetPreviewScene()->GetWorld()
				: nullptr;
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
#if 1
		UAGX_ShovelComponent* ToModify = Shovel;
#else
		UAGX_ShovelComponent* ToModify = FAGX_ShovelUtilities::GetShovelToModify(Shovel);
#endif
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
