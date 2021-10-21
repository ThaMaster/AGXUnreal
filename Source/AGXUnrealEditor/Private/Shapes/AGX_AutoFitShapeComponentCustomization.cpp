#include "Shapes/AGX_AutoFitShapeComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_AutoFitShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "FAGX_AutoFitShapeComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_AutoFitShapeComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AutoFitShapeComponentCustomization);
}

namespace AGX_AutoFitShapeComponentCustomization_helpers
{
	TArray<UStaticMeshComponent*> GetChildrenMeshComponents(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		TArray<UStaticMeshComponent*> Children;
		if (Blueprint == nullptr || Component == nullptr)
		{
			return Children;
		}

		TArray<USCS_Node*> Nodes = Blueprint->SimpleConstructionScript->GetAllNodes();
		USCS_Node* ComponentNode = Nodes.FindByPredicate(
			[Component](USCS_Node* Node) { return Node->ComponentTemplate == Component };);
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

			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child->ComponentTemplate))
			{
				Children.Add(Mesh);
			}
		}

		return Children;
	}

	FReply AutoFitChildInBlueprint(
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
				MeshOrigTransform.Add(Mesh, Mesh->GetComponentTransform());
				MeshesWithTransform.Add({Mesh->GetStaticMesh(), Mesh->GetComponentTransform()});
			}
		}

		if (MeshOrigTransform.Num() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		Component->AutoFit(MeshesWithTransform);

		// Update any archive instance with the new transform. Note: Using SetWorldTransform fails here
		// because that function internally uses the transform of the attach-parent to calculate a new
		// relative transform and sets that. Here, we might be dealing with "behind-the-scenes" objects
		// and for some reason these get garbage data from the attach-parent. Therefore we only use
		// SetRelative... functions on the instances.


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
				return AutoFitChildInBlueprint(Component, Blueprint);
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
