// Copyright 2022, Algoryx Simulation AB.


#include "Constraints/AGX_ConstraintCustomization.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintCustomization"

TSharedRef<IDetailCustomization> FAGX_ConstraintCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ConstraintCustomization);
}

void FAGX_ConstraintCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Fix category ordering (by priority AND by the order they edited).
	DetailBuilder.EditCategory(
		"AGX Constraint Bodies", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory(
		"AGX Constraint Dynamics", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory(
		"AGX Secondary Constraint", FText::GetEmpty(), ECategoryPriority::Important);
	DetailBuilder.EditCategory(
		"AGX Secondary Constraints", FText::GetEmpty(), ECategoryPriority::Important);
}

#undef LOCTEXT_NAMESPACE
