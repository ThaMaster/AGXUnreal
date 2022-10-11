// Copyright 2022, Algoryx Simulation AB.

#include "Widgets/AGX_ImportDialog.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "SAGX_ImportDialog"


void SAGX_ImportDialog::Construct(const FArguments& InArgs)
{
	// clang-format off
	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateBrowseFileGui()
				]
			]
		]
	];
	// clang-format on
}

namespace AGX_ImportDialog_helpers
{
	FSlateFontInfo CreateFont(int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	EAGX_ImportType GetFrom(const FString& FilePath)
	{
		const FString FileExtension = FPaths::GetExtension(FilePath);
		if (FileExtension.Equals("agx"))
		{
			return EAGX_ImportType::Agx;
		}
		else if (FileExtension.Equals("urdf"))
		{
			return EAGX_ImportType::Urdf;
		}

		return EAGX_ImportType::Invalid;
	}
}

TSharedRef<SWidget> SAGX_ImportDialog::CreateBrowseFileGui()
{
	using namespace AGX_ImportDialog_helpers;

	// clang-format off
return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BrowseFileText", "Select AGX Dynamics archive or URDF file"))
			.Font(CreateFont(12))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.f, 5.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FilePathText", "File:"))
				.Font(CreateFont(10))
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
			.AutoWidth()
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(150.0f)
				.Text(this, &SAGX_ImportDialog::GetFilePathText)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseButtonText", "Browse..."))
				.ToolTipText(LOCTEXT("BrowseButtonTooltip",
					"Browse to an AGX Dynamics archive or URDF file to import."))
				.OnClicked(this, &SAGX_ImportDialog::OnBrowseFileButtonClicked)
			]
		];
	// clang-format on
}

FReply SAGX_ImportDialog::OnBrowseFileButtonClicked()
{
	FilePath = FAGX_EditorUtilities::SelectExistingFileDialog(
		"All files", ".agx;*.urdf");
	ImportType = AGX_ImportDialog_helpers::GetFrom(FilePath);

	if (ImportType == EAGX_ImportType::Invalid)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Unable to detect file type for selected type.");
		FilePath = "";
		return FReply::Handled();
	}

	return FReply::Handled();
}

FText SAGX_ImportDialog::GetFilePathText() const
{
	return FText::FromString(FilePath);
}

FReply SAGX_ImportDialog::OnImportButtonClicked()
{
	if (FilePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"A AGX Dynamics archive or URDF file must be selected before importing.");
		return FReply::Handled();
	}

	TSharedRef<SWindow> ParentWindow =
		FSlateApplication::Get().FindWidgetWindow(AsShared()).ToSharedRef();
	FSlateApplication::Get().RequestDestroyWindow(ParentWindow);

	//AGX_ImporterToBlueprint::ImportAGXArchive(FilePath);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
