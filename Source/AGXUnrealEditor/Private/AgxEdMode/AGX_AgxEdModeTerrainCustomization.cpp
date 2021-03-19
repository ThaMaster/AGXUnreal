#include "AgxEdMode/AGX_AgxEdModeTerrainCustomization.h"

// AGX Dynamics includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_TerrainMaterialLibrary.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeTerrainCustomization"

TSharedRef<IDetailCustomization> FAGX_AgxEdModeTerrainCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AgxEdModeTerrainCustomization);
}


namespace FAGX_AgxEdModeTerrainCustomization_helpers
{
	void RefreshTerrainMaterialLibrary()
	{
		AGX_TerrainMaterialLibrary::InitializeTerrainMaterialAssetLibrary();
	}
}

void FAGX_AgxEdModeTerrainCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	using namespace FAGX_AgxEdModeTerrainCustomization_helpers;

	IDetailCategoryBuilder& CategoryBuilder =
		DetailBuilder.EditCategory("Terrain Material Library");
	CategoryBuilder.InitiallyCollapsed(false);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("btnRefreshTerrainMaterialLibrary", "Refresh Terrain Material Library"))
			.OnClicked_Lambda([]() { RefreshTerrainMaterialLibrary(); return FReply::Handled(); })
		]
	];
	// clang-format on
}




#undef LOCTEXT_NAMESPACE
