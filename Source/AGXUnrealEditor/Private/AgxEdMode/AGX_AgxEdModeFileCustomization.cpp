// Copyright 2021, Algoryx Simulation AB.


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

	// For now the two to-level import entries are hidden. Usability tests have shown that having
	// three options is too complicated for new users and we recommend using the to-Blueprint
	// version. Find another way to present the options so that the confusion is reduced. Possibly
	// using an import settings dialog. See internal issue 147.
#if 0
	AddCustomButton(
		CategoryBuilder,
		LOCTEXT(
			"CreateButtonTextImportLevelSingleActor",
			"Import AGX Dynamics Archive to level as a single actor..."),
		[&]() {
			UAGX_AgxEdModeFile::ImportAgxArchiveToSingleActor();
			return FReply::Handled();
		});
#endif

	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextImportBP", "Import AGX Archive to a Blueprint..."),
		[&]() {
			UAGX_AgxEdModeFile::ImportAgxArchiveToBlueprint();
			return FReply::Handled();
		});

	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextImportURDFBP", "Import URDF model to a Blueprint..."),
		[&]() {
			UAGX_AgxEdModeFile::ImportUrdfToBlueprint();
			return FReply::Handled();
		});
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileExporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Export AGX Archive");

	CategoryBuilder.InitiallyCollapsed(false);

	/** Create AGX Archive export Button */
	AddCustomButton(CategoryBuilder, LOCTEXT("CreateButtonTextEx", "Export AGX Archive..."), [&]() {
		UAGX_AgxEdModeFile::ExportAgxArchive();
		return FReply::Handled();
	});
}

#undef LOCTEXT_NAMESPACE
