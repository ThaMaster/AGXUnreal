#include "AGX_CollisionGroupManagerCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#include "AGX_LogCategory.h"
#include "AGX_ObjectUtilities.h"
#include "AGX_CollisionGroupManager.h"
#include "AGX_ShapeComponent.h"

#define LOCTEXT_NAMESPACE "FAGX_CollisionGroupManagerCustomization"

TSharedRef<IDetailCustomization> FAGX_CollisionGroupManagerCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CollisionGroupManagerCustomization);
}

void FAGX_CollisionGroupManagerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	AAGX_CollisionGroupManager* CollisionGroupManager =
		FAGX_ObjectUtilities::GetSingleObjectBeingCustomized<AAGX_CollisionGroupManager>(
			DetailBuilder);

	if (!CollisionGroupManager)
		return;

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Collision Group Manager");

	// Tell the CollisionGroupManager to update the collision
	// group list. This list will be visualized in the comboboxes.
	CollisionGroupManager->UpdateAvailableCollisionGroups();

	// Add first combobox
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock).Text(LOCTEXT("CollisionGroup1Name", "Collision Group 1"))]
		.ValueContent()
			[SNew(SComboBox<TSharedPtr<FName>>)
				 .ContentPadding(2)
				 .OptionsSource(&CollisionGroupManager->GetAvailableCollisionGroups())
				 .OnGenerateWidget_Lambda([=](TSharedPtr<FName> Item) {
					 // content for each item in combo box
					 return SNew(STextBlock)
						 .Text(FText::FromName(*Item))
						 .ToolTipText(FText::GetEmpty());
				 })
				 .OnSelectionChanged(
					 this, &FAGX_CollisionGroupManagerCustomization::OnConstraintTypeComboBoxChanged,
					 CollisionGroupManager, &CollisionGroupManager->GetSelectedGroup1())
				 .Content() // header content (i.e. showing selected item, even while combo box is
							// closed)
					 [SNew(STextBlock).Text_Lambda([CollisionGroupManager]() {
						 return FText::FromName(CollisionGroupManager->GetSelectedGroup1());
					 })]];

	// Add second combobox
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock).Text(LOCTEXT("CollisionGroup2Name", "Collision Group 2"))]
		.ValueContent()
			[SNew(SComboBox<TSharedPtr<FName>>)
				 .ContentPadding(2)
				 .OptionsSource(&CollisionGroupManager->GetAvailableCollisionGroups())
				 .OnGenerateWidget_Lambda([=](TSharedPtr<FName> Item) {
					 // content for each item in combo box
					 return SNew(STextBlock)
						 .Text(FText::FromName(*Item))
						 .ToolTipText(FText::GetEmpty());
				 })
				 .OnSelectionChanged(
					 this, &FAGX_CollisionGroupManagerCustomization::OnConstraintTypeComboBoxChanged,
					 CollisionGroupManager, &CollisionGroupManager->GetSelectedGroup2())
				 .Content() // header content (i.e. showing selected item, even while combo box is
							// closed)
					 [SNew(STextBlock).Text_Lambda([CollisionGroupManager]() {
						 return FText::FromName(CollisionGroupManager->GetSelectedGroup2());
					 })]];

	// Add button for disabling collision of selected collision groups
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) +
		 SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												.Text(LOCTEXT(
													"CreateCollisionDisGroupButtonText",
													"Disable collision"))
												.ToolTipText(LOCTEXT(
													"CreateCollisionDisGroupButtonTooltip",
													"Disable collision between selected groups."))
												.OnClicked_Lambda([CollisionGroupManager]() {
													CollisionGroupManager
														->DisableSelectedCollisionGroupPairs();
													return FReply::Handled();
												})]];

	// Add button for re-enable collision of selected collision groups
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) +
		 SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												.Text(LOCTEXT(
													"CreateCollisionEnaGroupButtonText",
													"Re-enable collision"))
												.ToolTipText(LOCTEXT(
													"CreateCollisionEnaGroupButtonTooltip",
													"Re-enable collision between selected groups."))
												.OnClicked_Lambda([CollisionGroupManager]() {
													CollisionGroupManager
														->ReenableSelectedCollisionGroupPairs();
													return FReply::Handled();
												})]];

	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AAGX_CollisionGroupManager, DisabledCollisionGroups)));
}

void FAGX_CollisionGroupManagerCustomization::OnConstraintTypeComboBoxChanged(
	TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
	AAGX_CollisionGroupManager* CollisionGroupManager, FName* SelectedGroup)
{
	if (CollisionGroupManager)
	{
		*SelectedGroup = *NewSelectedItem;
	}
}

#undef LOCTEXT_NAMESPACE
