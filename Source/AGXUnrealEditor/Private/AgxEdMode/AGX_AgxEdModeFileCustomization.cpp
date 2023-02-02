// Copyright 2023, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeFileCustomization.h"

#include "AgxEdMode/AGX_AgxEdModeFile.h"
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
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileImporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX File Importer");
	CategoryBuilder.InitiallyCollapsed(false);

	// Create import Buttons.

	AddCustomButton(
		CategoryBuilder,
		LOCTEXT("CreateButtonTextImportBP", "Import model to Blueprint..."),
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
		[&]()
		{
			UAGX_AgxEdModeFile::ExportAgxArchive();
			return FReply::Handled();
		});
}

#undef LOCTEXT_NAMESPACE
