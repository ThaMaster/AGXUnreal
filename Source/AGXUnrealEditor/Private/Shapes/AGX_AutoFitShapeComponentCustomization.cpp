#include "Shapes/AGX_AutoFitShapeComponentCustomization.h"

// AGX Dynamics for Unreal includes.
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
	FReply AutoFitInBlueprint(
		UAGX_AutoFitShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		// TODO: impl
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

	IDetailCategoryBuilder& CategoryBuilder =
		DetailBuilder.EditCategory("AGX Shape Auto-fit");

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
