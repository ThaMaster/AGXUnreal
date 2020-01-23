#include "AGX_CollisionGroupsComponentCustomization.h"

#include "CollisionGroups/AGX_CollisionGroupsComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_CollisionGroupsComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_CollisionGroupsComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CollisionGroupsComponentCustomization);
}

void FAGX_CollisionGroupsComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_CollisionGroupsComponent* CollisionGroupComponent =
		FAGX_ObjectUtilities::GetSingleObjectBeingCustomized<UAGX_CollisionGroupsComponent>(
			DetailBuilder);

	if (!CollisionGroupComponent)
		return;

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Collision Groups");

	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupsComponent, CollisionGroups)));

	// Add button for forcing a refresh of all child shape
	// components according to collision groups list
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) +
		 SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												.Text(LOCTEXT(
													"CreateCollisionForceRefreshButtonText",
													"Force refresh all shapes"))
												.ToolTipText(LOCTEXT(
													"CollisionForceRefreshButtonTooltip",
													"Force apply collision groups to all child "
													"shape components."))
												.OnClicked_Lambda([CollisionGroupComponent]() {
													CollisionGroupComponent
														->ForceRefreshChildShapes();
													return FReply::Handled();
												})]];

	// Hide CollisionGroupsLastChange from the Editor view
	DetailBuilder.HideProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupsComponent, CollisionGroupsLastChange)));
}

#undef LOCTEXT_NAMESPACE
