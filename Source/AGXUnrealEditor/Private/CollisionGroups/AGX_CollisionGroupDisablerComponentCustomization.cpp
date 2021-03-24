#include "CollisionGroups/AGX_CollisionGroupDisablerComponentCustomization.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"

#define LOCTEXT_NAMESPACE "FAGX_CollisionGroupDisablerComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_CollisionGroupDisablerComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CollisionGroupDisablerComponentCustomization);
}

void FAGX_CollisionGroupDisablerComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_CollisionGroupDisablerComponent>(
			DetailBuilder);

	if (!CollisionGroupDisabler)
		return;

	IDetailCategoryBuilder& CategoryBuilder =
		DetailBuilder.EditCategory("AGX Collision Group Pairs");

	// Tell the CollisionGroupDisabler to update the collision
	// group list. This list will be visualized in the comboboxes.
	CollisionGroupDisabler->UpdateAvailableCollisionGroups();

	UpdateAvailableCollisionGroups(CollisionGroupDisabler);

	// Add first combobox
	AddComboBox(
		CategoryBuilder, CollisionGroupDisabler,
		LOCTEXT("CollisionGroup1Name", "Collision Group 1"),
		&CollisionGroupDisabler->GetSelectedGroup1());

	// Add second combobox
	AddComboBox(
		CategoryBuilder, CollisionGroupDisabler,
		LOCTEXT("CollisionGroup2Name", "Collision Group 2"),
		&CollisionGroupDisabler->GetSelectedGroup2());

	// clang-format off

	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		// Add button for disabling collision of selected collision groups
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateCollisionDisGroupButtonText", "Disable collision"))
			.ToolTipText(LOCTEXT(
				"CreateCollisionDisGroupButtonTooltip",
				"Disable collision between selected groups."))
			.OnClicked_Lambda([CollisionGroupDisabler]() {
				const FScopedTransaction Transaction(LOCTEXT("CreateCollisionDisGroupUndo", "Disable collision group pair"));
				CollisionGroupDisabler->Modify();
				CollisionGroupDisabler->DisableSelectedCollisionGroupPairs();
				return FReply::Handled();
			})
		]

		// Add button for re-enable collision of selected collision groups
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateCollisionEnaGroupButtonText", "Re-enable collision"))
			.ToolTipText(LOCTEXT(
				"CreateCollisionEnaGroupButtonTooltip",
				"Re-enable collision between selected groups."))
			.OnClicked_Lambda([CollisionGroupDisabler]() {
				const FScopedTransaction Transaction(LOCTEXT("CreateCollisionEnaGroupUndo", "Re-enable collision group pair"));
				CollisionGroupDisabler->Modify();
				CollisionGroupDisabler->ReenableSelectedCollisionGroupPairs();
				return FReply::Handled();
			})
		]
	];

	// Add button for clearing invalid collision group pairs.
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RemoveInvalidGroupsPairsButtonText", "Remove invalid group pairs"))
			.ToolTipText(LOCTEXT(
				"RemoveInvalidGroupPairsButtonTooltip",
				"Remove all collision group pairs that contains collision groups that no shape has."))
			.OnClicked_Lambda([CollisionGroupDisabler]() {
				const FScopedTransaction Transaction(LOCTEXT("RemoveInvalidGroupPairsUndo", "Remove invalid group pairs"));
				CollisionGroupDisabler->Modify();
				CollisionGroupDisabler->UpdateAvailableCollisionGroupsFromWorld();
				CollisionGroupDisabler->RemoveDeprecatedCollisionGroups();
				return FReply::Handled();
			})
		]
	];

	// clang-format on

	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(
		UAGX_CollisionGroupDisablerComponent, DisabledCollisionGroupPairs)));
}

void FAGX_CollisionGroupDisablerComponentCustomization::OnComboBoxChanged(
	TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler, FName* SelectedGroup)
{
	if (CollisionGroupDisabler)
	{
		*SelectedGroup = *NewSelectedItem;
	}
}

void FAGX_CollisionGroupDisablerComponentCustomization::UpdateAvailableCollisionGroups(
	const UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler)
{
	AvailableCollisionGroups.Empty();

	for (FName CollisionGroups : CollisionGroupDisabler->GetAvailableCollisionGroups())
		AvailableCollisionGroups.Add(MakeShareable(new FName(CollisionGroups)));
}

void FAGX_CollisionGroupDisablerComponentCustomization::AddComboBox(
	IDetailCategoryBuilder& CategoryBuilder,
	UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler, FText Name, FName* SelectedGroup)
{
	//clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	.NameContent()
	[
		SNew(STextBlock).Text(Name)
	]
	.ValueContent()
	[
		SNew(SComboBox<TSharedPtr<FName>>)
		.ContentPadding(2)
		.OptionsSource(&AvailableCollisionGroups)
		.OnGenerateWidget_Lambda([=](TSharedPtr<FName> Item)
		 {
			// content for each item in combo box
			return SNew(STextBlock)
			.Text(FText::FromName(*Item))
			.ToolTipText(FText::GetEmpty());
		})
		.OnSelectionChanged(
			this, &FAGX_CollisionGroupDisablerComponentCustomization::OnComboBoxChanged,
			CollisionGroupDisabler, SelectedGroup)
		.Content() // header content showing selected item, even while combo box is closed.
		[
			SNew(STextBlock)
			.Text_Lambda([SelectedGroup]()
			{
				return FText::FromName(*SelectedGroup);
			})
		]
	];
	//clang-format on
}

#undef LOCTEXT_NAMESPACE
