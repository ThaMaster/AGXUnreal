#include "Shapes/AGX_AutoFitShapeComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_AutoFitShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/SCS_Node.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "FAGX_AutoFitShapeComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_AutoFitShapeComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AutoFitShapeComponentCustomization);
}

namespace AGX_AutoFitShapeComponentCustomization_helpers
{
	USCS_Node* GetSCSNodeFromComponent(
		USceneComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return nullptr;
		}

		TArray<USCS_Node*> Nodes = Blueprint->SimpleConstructionScript->GetAllNodes();
		USCS_Node** ComponentNode = Nodes.FindByPredicate(
			[Component](USCS_Node* Node) { return Node->ComponentTemplate == Component; });
		if (ComponentNode == nullptr)
		{
			return nullptr;
		}

		return *ComponentNode;
	}

	FTransform GetBlueprintComponentWorldTransform(
		USceneComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FTransform::Identity;
		}

		USCS_Node* ComponentNode = GetSCSNodeFromComponent(Component, Blueprint);
		if (ComponentNode == nullptr)
		{
			return FTransform::Identity;
		}

		// Build a chain of USCS_Nodes starting from root and going down to the Component's
		// USCS_Node.
		TArray<USCS_Node*> RootToComponentChain;
		USCS_Node* CurrentNode = ComponentNode;
		RootToComponentChain.Insert(CurrentNode, 0);
		while (USCS_Node* Parent = Blueprint->SimpleConstructionScript->FindParentNode(CurrentNode))
		{
			RootToComponentChain.Insert(Parent, 0);
			CurrentNode = Parent;
		}

		FTransform WorldTransform = FTransform::Identity;
		for (USCS_Node* Node : RootToComponentChain)
		{
			if (Node == nullptr || Node->ComponentTemplate == nullptr)
			{
				continue;
			}
			if (USceneComponent* SceneComponent = Cast<USceneComponent>(Node->ComponentTemplate))
			{
				const FTransform RelativeTransform = SceneComponent->GetRelativeTransform();
				FTransform::Multiply(&WorldTransform, &RelativeTransform, &WorldTransform);
			}
		}

		return WorldTransform;
	}

	void SetBlueprintComponentWorldTransform(
		USceneComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const FTransform& Transform)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return;
		}

		USCS_Node* ComponentNode = GetSCSNodeFromComponent(Component, Blueprint);
		if (ComponentNode == nullptr)
		{
			return;
		}

		USCS_Node* ParentNode = Blueprint->SimpleConstructionScript->FindParentNode(ComponentNode);
		if (ParentNode == nullptr || ParentNode->ComponentTemplate == nullptr)
		{
			return;
		}

		USceneComponent* ParentSceneComponent =
			Cast<USceneComponent>(ParentNode->ComponentTemplate);
		if (ParentSceneComponent == nullptr)
		{
			return;
		}

		const FTransform ParentWorldTransform =
			GetBlueprintComponentWorldTransform(ParentSceneComponent, Blueprint);
		const FTransform NewRelTransform = Transform.GetRelativeTransform(ParentWorldTransform);
		Component->Modify();
		Component->SetRelativeTransform(NewRelTransform);
	}

	TArray<UStaticMeshComponent*> GetChildrenMeshComponents(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		TArray<UStaticMeshComponent*> Children;
		if (Blueprint == nullptr || Component == nullptr)
		{
			return Children;
		}

		TArray<USCS_Node*> Nodes = Blueprint->SimpleConstructionScript->GetAllNodes();
		USCS_Node** ComponentNode = Nodes.FindByPredicate(
			[Component](USCS_Node* Node) { return Node->ComponentTemplate == Component; });
		if (ComponentNode == nullptr || *ComponentNode == nullptr)
		{
			return Children;
		}

		TArray<USCS_Node*> ChildNodes = (*ComponentNode)->GetChildNodes();
		for (USCS_Node* Child : ChildNodes)
		{
			if (Child == nullptr)
			{
				continue;
			}

			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child->ComponentTemplate))
			{
				Children.Add(Mesh);
			}
		}

		return Children;
	}

	void UpdateTransformOfArchiveInstances(
		USceneComponent* Component, const FVector& OrigRelLocation, const FRotator& OrigRelRotation)
	{
		if (Component == nullptr)
		{
			return;
		}

		// Note: Using SetWorldTransform fails here because that function internally uses the
		// transform of the attach-parent to calculate a new relative transform and sets that. When
		// dealing with objects inside a Blueprint, the attach-parent is not set. Therefore we must
		// stick to using only RelativeLocation/Rotation.
		for (USceneComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only write to the Archetype Instances if they are currently in sync with this
			// template.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation)
			{
				Instance->Modify();
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// The purpose of this function is to make sure the Instances get exactly the same
				// relative transform as the Archetype. However, SetRelativeLocation/Rotation does
				// some transformation calculations internally which in some cases result in (small)
				// rounding errors, which is enough to break the state in the Blueprint. We call the
				// Set..._Direct functions here to ensure that the RelativeLocation/Rotation matches
				// exactly with the archetype. The above calls are still needed because those make
				// sure the component is updated in the viewport without the need to recompile the
				// Blueprint.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}
	}

	void AutoFitBox(UAGX_BoxShapeComponent* Component, const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr)
		{
			return;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const FVector OrigHalfExtent = Component->GetHalfExtent();

		Component->Modify();
		Component->AutoFit(Meshes);

		// Update any archetype instance in need of update.
		for (UAGX_BoxShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetHalfExtent() == OrigHalfExtent)
			{
				Instance->Modify();
				Instance->SetHalfExtent(Component->GetHalfExtent());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// See comment in UpdateTransformOfArchiveInstances.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}
	}

	void AutoFitCylinder(
		UAGX_CylinderShapeComponent* Component, const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr)
		{
			return;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		Component->AutoFit(Meshes);

		// Update any archetype instance in need of update.
		for (UAGX_CylinderShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetRadius() == OrigRadius && Instance->GetHeight() == OrigHeight)
			{
				Instance->Modify();
				Instance->SetRadius(Component->GetRadius());
				Instance->SetHeight(Component->GetHeight());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// See comment in UpdateTransformOfArchiveInstances.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}
	}

	void AutoFitCapsule(
		UAGX_CapsuleShapeComponent* Component, const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr)
		{
			return;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		Component->AutoFit(Meshes);

		// Update any archetype instance in need of update.
		for (UAGX_CapsuleShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetRadius() == OrigRadius && Instance->GetHeight() == OrigHeight)
			{
				Instance->Modify();
				Instance->SetRadius(Component->GetRadius());
				Instance->SetHeight(Component->GetHeight());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// See comment in UpdateTransformOfArchiveInstances.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}
	}

	FReply AutoFitToChildInBlueprint(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		TArray<UStaticMeshComponent*> MeshChildren =
			GetChildrenMeshComponents(Component, Blueprint);
		TMap<UStaticMeshComponent*, FTransform> MeshOrigTransform;
		TArray<FAGX_MeshWithTransform> MeshesWithTransform;
		for (UStaticMeshComponent* Mesh : MeshChildren)
		{
			if (Mesh != nullptr)
			{
				const FTransform WorldTransform =
					GetBlueprintComponentWorldTransform(Mesh, Blueprint);
				MeshOrigTransform.Add(Mesh, WorldTransform);
				MeshesWithTransform.Add({Mesh->GetStaticMesh(), WorldTransform});
			}
		}

		if (MeshOrigTransform.Num() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		const FScopedTransaction Transaction(
			LOCTEXT("AutoFitChildrenBPUndo", "Undo Auto-fit operation"));

		if (UAGX_BoxShapeComponent* Box = Cast<UAGX_BoxShapeComponent>(Component))
		{
			AutoFitBox(Box, MeshesWithTransform);
		}
		else if (
			UAGX_CylinderShapeComponent* Cylinder = Cast<UAGX_CylinderShapeComponent>(Component))
		{
			AutoFitCylinder(Cylinder, MeshesWithTransform);
		}
		else if (UAGX_CapsuleShapeComponent* Capsule = Cast<UAGX_CapsuleShapeComponent>(Component))
		{
			AutoFitCapsule(Capsule, MeshesWithTransform);
		}
		else
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unknown Auto Fit Shape Component type passed to AutoFitToChildInBlueprint."));
			return FReply::Handled();
		}

		// Finally, we need to restore the children meshes to their original world location.
		for (UStaticMeshComponent* MeshChild : MeshChildren)
		{
			if (MeshChild == nullptr)
			{
				continue;
			}

			const FVector MeshOrigLocation = MeshChild->GetRelativeLocation();
			const FRotator MeshOrigRotation = MeshChild->GetRelativeRotation();
			SetBlueprintComponentWorldTransform(MeshChild, Blueprint, MeshOrigTransform[MeshChild]);
			UpdateTransformOfArchiveInstances(MeshChild, MeshOrigLocation, MeshOrigRotation);
		}

		return FReply::Handled();
	}

	FReply AutoFitInBlueprint(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		switch (Component->MeshSourceLocation)
		{
			case EAGX_StaticMeshSourceLocation::TSL_PARENT_STATIC_MESH_COMPONENT:
				// TODO: impl
				return FReply::Handled();
			case EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT:
				return AutoFitToChildInBlueprint(Component, Blueprint);
			case EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET:
				// TODO: impl
				return FReply::Handled();
		}

		UE_LOG(LogAGX, Error, TEXT("Unknown MeshSourceLocation given to AutoFitInBlueprint."));
		return FReply::Handled();
	}

	FReply OnAutoFitButtonClicked(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_AutoFitShapeComponent* AutoFitShapeComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_AutoFitShapeComponent>(
				DetailBuilder);

		if (!AutoFitShapeComponent)
		{
			return FReply::Handled();
		}

		if (AutoFitShapeComponent->IsInBlueprint())
		{
			return AutoFitInBlueprint(
				AutoFitShapeComponent,
				Cast<UBlueprintGeneratedClass>(AutoFitShapeComponent->GetOuter()));
		}

		const FScopedTransaction Transaction(LOCTEXT("AutoFitUndo", "Undo Auto-fit operation"));
		AutoFitShapeComponent->Modify();
		AutoFitShapeComponent->AutoFitFromSelection();

		return FReply::Handled();
	}
}

void FAGX_AutoFitShapeComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_AutoFitShapeComponent* AutoFitShapeComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_AutoFitShapeComponent>(
			DetailBuilder);

	if (!AutoFitShapeComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Shape Auto-fit");

	// clang-format off

	// Add auto-fit button.
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("AutoFitButtonText", "Auto-fit to Static Mesh"))
			.ToolTipText(LOCTEXT(
				"AutoFitButtonTooltip",
				"Auto-fit this Shape to the Static Meshs(es) given by the current Mesh Source Location."))
			.OnClicked_Lambda([&DetailBuilder]() {
				return AGX_AutoFitShapeComponentCustomization_helpers::OnAutoFitButtonClicked(DetailBuilder);
			})
		]
	];

	// clang-format on
}

#undef LOCTEXT_NAMESPACE
