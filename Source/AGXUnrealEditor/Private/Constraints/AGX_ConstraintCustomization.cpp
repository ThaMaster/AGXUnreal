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

	// Fix category ordering by priority by the order they are often edited.
	IDetailCategoryBuilder& BodiesCategory = DetailBuilder->EditCategory(
		"AGX Constraint Bodies", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory("AGX Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder->EditCategory(
		"AGX Secondary Constraints", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off
	BodiesCategory.AddCustomRow(LOCTEXT("Error","Error"))
	[
		// Switch to SListView and populate a list of FTexts if need more performance.
		SNew(STextBlock)
		.Text(this, &FAGX_ConstraintCustomization::GetBodySetupErrorText)
		.ToolTipText(this, &FAGX_ConstraintCustomization::GetBodySetupErrorText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ColorAndOpacity(FLinearColor::Red)
	]
	.Visibility(TAttribute<EVisibility>(
		this, &FAGX_ConstraintCustomization::VisibleWhenBodySetupError));
	// clang-format on
}

namespace AGX_ConstraintCustomization_helpers
{
	enum class EBodySetupError : int
	{
		NoError = 0,
		NoFirstBody = 1 << 0,
		SameBody = 1 << 1
	};

	EBodySetupError& operator|=(EBodySetupError& InOutLhs, EBodySetupError InRhs)
	{
		int Lhs = (int) InOutLhs;
		int Rhs = (int) InRhs;
		int result = Lhs | Rhs;
		InOutLhs = (EBodySetupError) result;
		return InOutLhs;
	}

	EBodySetupError operator&(EBodySetupError InLhs, EBodySetupError InRhs)
	{
		int Lhs = (int) InLhs;
		int Rhs = (int) InRhs;
		int Result = Lhs & Rhs;
		return (EBodySetupError) Result;
	}

	bool HasError(EBodySetupError Error, EBodySetupError Flag)
	{
		return (Error & Flag) != EBodySetupError::NoError;
	}

	EBodySetupError GetBodySetupError(const IDetailLayoutBuilder& DetailBuilder)
	{
		EBodySetupError Error {EBodySetupError::NoError};

		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		for (auto& Object : Objects)
		{
			UAGX_ConstraintComponent* Constraint = Cast<UAGX_ConstraintComponent>(Object.Get());
			if (Constraint == nullptr)
				continue;

			UAGX_RigidBodyComponent* Body1 = Constraint->BodyAttachment1.GetRigidBody();
			UAGX_RigidBodyComponent* Body2 = Constraint->BodyAttachment2.GetRigidBody();

			if (Body1 == nullptr)
			{
				Error |= EBodySetupError::NoFirstBody;
			}

			if (Body1 != nullptr && Body1 == Body2)
			{
				Error |= EBodySetupError::SameBody;
			}
		}

		return Error;
	}
}

EVisibility FAGX_ConstraintCustomization::VisibleWhenBodySetupError() const
{
	using namespace AGX_ConstraintCustomization_helpers;
	if (DetailBuilder == nullptr)
		return EVisibility::Collapsed;
	EBodySetupError Error = GetBodySetupError(*DetailBuilder);
	return FAGX_EditorUtilities::VisibleIf(Error != EBodySetupError::NoError);
}

FText FAGX_ConstraintCustomization::GetBodySetupErrorText() const
{
	using namespace AGX_ConstraintCustomization_helpers;
	if (DetailBuilder == nullptr)
		return FText();

	EBodySetupError Error = GetBodySetupError(*DetailBuilder);
	FString Message;
	auto Append = [&Message](const FText& Text)
	{
		if (!Message.IsEmpty())
			Message += TEXT("\n");
		Message += Text.ToString();
	};

	if (HasError(Error, EBodySetupError::NoFirstBody))
	{
		static FText Text = LOCTEXT("NoFirstBody", "A Constraint must have a first Rigid Body .");
		Append(Text);
	}
	if (HasError(Error, EBodySetupError::SameBody))
	{
		static FText Text = LOCTEXT(
			"BothBodiesSame", "Both Body Attachments may not reference the same Rigid Body");
		Append(Text);
	}

	return FText::FromString(Message);
}

#undef LOCTEXT_NAMESPACE
