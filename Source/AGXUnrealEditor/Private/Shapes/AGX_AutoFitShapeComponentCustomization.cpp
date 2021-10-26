#include "Shapes/AGX_AutoFitShapeComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_AutoFitShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
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
	UStaticMeshComponent* GetParentMeshComponent(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return nullptr;
		}

		USCS_Node* Current = FAGX_BlueprintUtilities::GetSCSNodeFromComponent(Component);
		if (Current == nullptr)
		{
			return nullptr;
		}

		while (USCS_Node* Parent = Blueprint->SimpleConstructionScript->FindParentNode(Current))
		{
			if (UStaticMeshComponent* S = Cast<UStaticMeshComponent>(Parent->ComponentTemplate))
			{
				return S;
			}
			Current = Parent;
		}

		return nullptr;
	}

	TArray<UStaticMeshComponent*> GetChildrenMeshComponents(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		TArray<UStaticMeshComponent*> Children;
		if (Blueprint == nullptr || Component == nullptr)
		{
			return Children;
		}

		USCS_Node* ComponentNode = FAGX_BlueprintUtilities::GetSCSNodeFromComponent(Component);
		if (ComponentNode == nullptr)
		{
			return Children;
		}

		TArray<USCS_Node*> ChildNodes = ComponentNode->GetChildNodes();
		for (USCS_Node* Child : ChildNodes)
		{
			if (Child == nullptr)
			{
				continue;
			}

			// Only include immediate children.
			USCS_Node* Parent = Blueprint->SimpleConstructionScript->FindParentNode(Child);
			if (Parent != ComponentNode)
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

	bool AutoFitBox(
		UAGX_BoxShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const FVector OrigHalfExtent = Component->GetHalfExtent();

		Component->Modify();
		if (!Component->AutoFit(Meshes))
		{
			// Logging done in AutoFit.
			return false;
		}

		// The following is a little ugly, but necessary. Component->AutoFit will set a new world
		// transform on the Component, but world transform is not handled correctly by template
		// components inside a Blueprint. In actuality, SetWorldTransform will behave as if the
		// component is in world origin, i.e. the same way as SetRelativeTransform. Therefore, we
		// use the SetTemplateComponentWorldTransform which correctly handles the world transform
		// of template components.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

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

				// The purpose of this for-loop is to make sure the Instances get exactly the same
				// relative transform as the archetype. However, SetRelativeLocation/Rotation does
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

		return true;
	}

	bool AutoFitCylinder(
		UAGX_CylinderShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		if (!Component->AutoFit(Meshes))
		{
			// Logging done in AutoFit.
			return false;
		}

		// See comment in AutoFitBox.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

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

				// See comment in AutoFitBox.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}

		return true;
	}

	bool AutoFitCapsule(
		UAGX_CapsuleShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		if (!Component->AutoFit(Meshes))
		{
			// Logging done in AutoFit.
			return false;
		}

		// See comment in AutoFitBox.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

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

				// See comment in AutoFitBox.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}

		return true;
	}

	bool AutoFitAny(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		if (UAGX_BoxShapeComponent* Box = Cast<UAGX_BoxShapeComponent>(Component))
		{
			return AutoFitBox(Box, Blueprint, Meshes);
		}
		else if (
			UAGX_CylinderShapeComponent* Cylinder = Cast<UAGX_CylinderShapeComponent>(Component))
		{
			return AutoFitCylinder(Cylinder, Blueprint, Meshes);
		}
		else if (UAGX_CapsuleShapeComponent* Capsule = Cast<UAGX_CapsuleShapeComponent>(Component))
		{
			return AutoFitCapsule(Capsule, Blueprint, Meshes);
		}
		else
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unknown Auto Fit Shape Component type passed to AutoFitToChildInBlueprint."));
			return false;
		}
	}

	FReply AutoFitToAssetInBlueprint(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		UStaticMesh* Asset = Component->MeshSourceAsset;
		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		const FTransform WorldTransform =
			FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(Component);
		FAGX_MeshWithTransform Mesh(Asset, WorldTransform);

		// Logging done in AutoFitAny.
		return FReply::Handled();
	}

	FReply AutoFitToParentInBlueprint(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		UStaticMeshComponent* MeshParent = GetParentMeshComponent(Component, Blueprint);
		if (MeshParent == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		const FTransform WorldTransform =
			FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(MeshParent);
		FAGX_MeshWithTransform Mesh(MeshParent->GetStaticMesh(), WorldTransform);

		// Logging done in AutoFitAny.
		return FReply::Handled();
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
					FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(Mesh);
				MeshOrigTransform.Add(Mesh, WorldTransform);
				MeshesWithTransform.Add({Mesh->GetStaticMesh(), WorldTransform});
			}
		}

		if (MeshesWithTransform.Num() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		if (!AutoFitAny(Component, Blueprint, MeshesWithTransform))
		{
			// Logging done in AutoFitAny.
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
			FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
				MeshChild, MeshOrigTransform[MeshChild]);
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
				return AutoFitToParentInBlueprint(Component, Blueprint);
			case EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT:
				return AutoFitToChildInBlueprint(Component, Blueprint);
			case EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET:
				return AutoFitToAssetInBlueprint(Component, Blueprint);
		}

		UE_LOG(LogAGX, Error, TEXT("Unknown MeshSourceLocation given to AutoFitInBlueprint."));
		return FReply::Handled();
	}

	struct FScopedProperyNotify
	{
		FScopedProperyNotify() = delete;
		FScopedProperyNotify(TArray<TSharedRef<IPropertyHandle>>& InProperties)
			: Properties(InProperties)
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (!Handle->IsValidHandle())
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("FScopedProperyNotify was given an invalid propery handle."));
					continue;
				}

				Handle->NotifyPreChange();
			}
		}

		~FScopedProperyNotify()
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (Handle->IsValidHandle())
				{
					Handle->NotifyPostChange();
				}
			}
		}

		private:
		const TArray<TSharedRef<IPropertyHandle>>& Properties;
	};

	FReply OnAutoFitButtonClicked(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_AutoFitShapeComponent* AutoFitShapeComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_AutoFitShapeComponent>(
				DetailBuilder);

		if (!AutoFitShapeComponent)
		{
			return FReply::Handled();
		}

		TArray<TSharedRef<IPropertyHandle>> Properties;
		Properties.Add(
			DetailBuilder.GetProperty(FName("RelativeLocation"), USceneComponent::StaticClass()));
		Properties.Add(
			DetailBuilder.GetProperty(FName("RelativeRotation"), USceneComponent::StaticClass()));
		const FScopedProperyNotify PropertyNotify(Properties);
		const FScopedTransaction Transaction(LOCTEXT("AutoFitUndo", "Undo Auto-fit operation"));

		if (AutoFitShapeComponent->IsInBlueprint())
		{
			// Logging done in AutoFitInBlueprint.
			return AutoFitInBlueprint(
				AutoFitShapeComponent,
				Cast<UBlueprintGeneratedClass>(AutoFitShapeComponent->GetOuter()));
		}

		// Call Modify on children meshes if TSL_CHILD_STATIC_MESH_COMPONENT is used, to support
		// undo/redo.
		if (AutoFitShapeComponent->MeshSourceLocation ==
			EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT)
		{
			for (UStaticMeshComponent* Child :
				 AutoFitShapeComponent->FindImmediateChildrenMeshComponents())
			{
				if (Child == nullptr)
				{
					continue;
				}
				Child->Modify();
			}
		}
		AutoFitShapeComponent->Modify();
		AutoFitShapeComponent->AutoFitFromSelection();

		// Logging done in AutoFitFromSelection.
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
