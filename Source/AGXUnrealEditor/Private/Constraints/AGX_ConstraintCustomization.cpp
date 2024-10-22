// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintCustomization"

TSharedRef<IDetailCustomization> FAGX_ConstraintCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintCustomization);
}

void FAGX_ConstraintCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	// Fix category ordering (by priority AND by the order they edited).
	IDetailCategoryBuilder& BodiesCategory = DetailBuilder->EditCategory(
		"AGX Constraint Bodies", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraints", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off
	BodiesCategory.AddCustomRow(LOCTEXT("Error","Error"))
	[
		SNew(STextBlock)
		.Text(LOCTEXT("BothBodiesSame", "Both Body Attachments may not reference the same Rigid Body"))
		.ToolTipText(LOCTEXT("BothBodiesSame", "Both Body Attachments may not reference the same Rigid Body"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ColorAndOpacity(FLinearColor::Red)
	]
	.Visibility(TAttribute<EVisibility>(this, &FAGX_ConstraintCustomization::VisibleWhenSameBody));
	// clang-format on
}

EVisibility FAGX_ConstraintCustomization::VisibleWhenSameBody() const
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder->GetObjectsBeingCustomized(Objects);

	bool bAnyEqual {false};
	for (auto& Object : Objects)
	{
		UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(Object.Get());
		if (Constraint == nullptr)
			continue;

		UAGX_RigidBodyComponent* Body1 = Constraint->BodyAttachment1.GetRigidBody();
		UAGX_RigidBodyComponent* Body2 = Constraint->BodyAttachment2.GetRigidBody();
		if (Body1 != nullptr && Body1 == Body2)
		{
			bAnyEqual = true;
			break;
		}
	}

	return FAGX_EditorUtilities::VisibleIf(bAnyEqual);
}

#undef LOCTEXT_NAMESPACE
