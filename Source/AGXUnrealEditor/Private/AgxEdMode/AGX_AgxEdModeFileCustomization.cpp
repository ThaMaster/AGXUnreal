// Copyright 2023, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeFileCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_MaterialLibrary.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeFileCustomization"

TSharedRef<IDetailCustomization> FAGX_AgxEdModeFileCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AgxEdModeFileCustomization);
}

void FAGX_AgxEdModeFileCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizeFileImporterCategory(DetailBuilder);
	CustomizeFileExporterCategory(DetailBuilder);
	CustomizeMaterialLibraryCategory(DetailBuilder);
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileImporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX File Importer");
	CategoryBuilder.InitiallyCollapsed(false);

	// Create import Buttons.

	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextImportBP", "Import model to Blueprint..."),
		LOCTEXT("CreateButtonTextImportTt", "Import a model from a file to a Blueprint."),
		[&]()
		{
			UAGX_AgxEdModeFile::ImportToBlueprint();
			return FReply::Handled();
		});
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileExporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Export AGX Archive");

	CategoryBuilder.InitiallyCollapsed(false);

	/** Create AGX Archive export Button */
	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextEx", "Export AGX Archive..."),
		LOCTEXT(
			"CreateButtonTextTt", "Export the current Simulation state to an AGX Archive (.agx)."),
		[&]()
		{
			UAGX_AgxEdModeFile::ExportAgxArchive();
			return FReply::Handled();
		});
}

namespace AGX_AgxEdModeFileCustomization_helpers
{
	void RefreshMaterialLibraries()
	{
		AGX_MaterialLibrary::InitializeShapeMaterialAssetLibrary();
		AGX_MaterialLibrary::InitializeContactMaterialAssetLibrary();		
		AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary();
	}
}

void FAGX_AgxEdModeFileCustomization::CustomizeMaterialLibraryCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Material Library");

	CategoryBuilder.InitiallyCollapsed(false);

	/** Create AGX Naterial Libraries refresh Button */
	AddCustomButton(
		CategoryBuilder, LOCTEXT("RefreshMaterialLibraryEx", "Refresh Material Libraries"),
		LOCTEXT(
			"RefreshMaterialLibraryTt",
			"Reads all Materials in the AGX Dynamics Materials "
			"Library and updates the pre-defined Materials in the Plugin Contents."),
		[&]()
		{
			AGX_AgxEdModeFileCustomization_helpers::RefreshMaterialLibraries();
			FAGX_NotificationUtilities::ShowNotification(
				"Material Library Updated.", SNotificationItem::CS_None);
			return FReply::Handled();
		});
}

#undef LOCTEXT_NAMESPACE
