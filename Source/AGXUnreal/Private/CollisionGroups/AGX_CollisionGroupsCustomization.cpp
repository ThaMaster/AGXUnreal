#include "AGX_CollisionGroupsCustomization.h"

#include "AGX_CollisionGroups.h"
#include "AGX_ObjectUtilities.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_CollisionGroupsCustomization"

TSharedRef<IDetailCustomization> FAGX_CollisionGroupsCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CollisionGroupsCustomization);
}

void FAGX_CollisionGroupsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_CollisionGroups* CollisionGroupComponent =
		FAGX_ObjectUtilities::GetSingleObjectBeingCustomized<UAGX_CollisionGroups>(DetailBuilder);

	if (!CollisionGroupComponent)
		return;

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Collision Groups");

	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroups, CollisionGroups)));

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
}

#undef LOCTEXT_NAMESPACE
