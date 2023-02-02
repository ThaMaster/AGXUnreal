// Copyright 2023, Algoryx Simulation AB.

#include "AGX_SimulationCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Misc/Paths.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_SimulationCustomization"

TSharedRef<IDetailCustomization> FAGX_SimulationCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_SimulationCustomization);
}

void FAGX_SimulationCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(InDetailBuilder);

	if (Simulation == nullptr)
		return;

	// Hide the default ExportPath widget, we will create a custom one for it.
	InDetailBuilder.HideProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_Simulation, ExportPath)));

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Startup");

	CategoryBuilder.AddProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_Simulation, bExportInitialState)));

	// clang-format off

	// Create the widgets for browsing to an output file.	
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(3.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OutputFilePathText", "Output File:"))
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &FAGX_SimulationCustomization::GetOutputFilePathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseButtonText", "Browse..."))
					.ToolTipText(LOCTEXT("BrowseButtonTooltip",
						"Specify an output file for the initial state."))
					.OnClicked(this, &FAGX_SimulationCustomization::OnBrowseFileButtonClicked)
				]
			]
		]
	];
	// clang-format on
}

FText FAGX_SimulationCustomization::GetOutputFilePathText() const
{
	if (DetailBuilder == nullptr)
		return FText();

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
		return FText();

	// We hide paths that does not exist, since it may come from a different computer, saved to
	// the configuration .ini file of a project.
	if (!FPaths::DirectoryExists(FPaths::GetPath(Simulation->ExportPath)))
		return FText();

	return FText::FromString(Simulation->ExportPath);
}

FReply FAGX_SimulationCustomization::OnBrowseFileButtonClicked()
{
	if (DetailBuilder == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithWarningLog(
			"Unexpected error, unable to get the Detail Builder. Please contact the Algoryx "
			"support.");
		return FReply::Handled();
	}

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithWarningLog(
			"Unexpected error, unable to get the Simulation object. Please contact the Algoryx "
			"support.");
		return FReply::Handled();
	}

	const FString StartDir = [Simulation]()
	{
		const FString DirPath = FPaths::GetPath(Simulation->ExportPath);
		return FPaths::DirectoryExists(DirPath) ? DirPath : FString("");
	}();

	FString OutputFilePath = FAGX_EditorUtilities::SelectNewFileDialog(
		"Output file", "AGX Dynamics Archive|*.agx", "AGXUnreal", StartDir);

	if (OutputFilePath.IsEmpty())
		return FReply::Handled();

	Simulation->ExportPath = OutputFilePath;
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
