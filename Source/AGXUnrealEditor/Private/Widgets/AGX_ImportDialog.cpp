// Copyright 2022, Algoryx Simulation AB.

#include "Widgets/AGX_ImportDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImportSettings.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Misc/FileHelper.h"
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
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 0.0f))
			.AutoHeight()
			[
				CreateImportAGXFileGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 0.0f))
			.AutoHeight()
			[
				CreateImportURDFFileGui()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				CreateSettingsGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				CreateImportButtonGui()
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

	bool UrdfHasFilenameAttribute(const FString& FilePath)
	{
		FString Content;
		if (!FFileHelper::LoadFileToString(Content, *FilePath))
		{
			UE_LOG(LogAGX, Warning, TEXT("Unable to read file '%s'"), *FilePath);
			return false;
		}

		return Content.Contains("filename", ESearchCase::IgnoreCase);
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

TSharedRef<SBorder> SAGX_ImportDialog::CreateImportButtonGui()
{
	if (ImportType == EAGX_ImportType::Invalid)
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					SNew(SHorizontalBox)		
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("ImportButtonText", "Import"))
						.ToolTipText(LOCTEXT("ImportButtonTooltip",
							"Import the selected file with the specified settings to a Blueprint."))
						.OnClicked(this, &SAGX_ImportDialog::OnImportButtonClicked)
					]
				];

	// clang-format on
}

TSharedRef<SBorder> SAGX_ImportDialog::CreateSettingsGui()
{
	if (ImportType == EAGX_ImportType::Invalid)
	{
		return MakeShared<SBorder>();
	}

	using namespace AGX_ImportDialog_helpers;

	// clang-format off
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)	
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SettingsText", "Settings"))
				.Font(CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				CreateCheckboxGui()
			]			
		];
	// clang-format on
}

TSharedRef<SBorder> SAGX_ImportDialog::CreateImportAGXFileGui()
{
	return MakeShared<SBorder>();
}

TSharedRef<SBorder> SAGX_ImportDialog::CreateImportURDFFileGui()
{
	if (ImportType != EAGX_ImportType::Urdf)
	{
		return MakeShared<SBorder>();
	}

	using namespace AGX_ImportDialog_helpers;
	if (!UrdfHasFilenameAttribute(FilePath))
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(10.0f, 5.0f))
				.Content()
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("BrowseUrdfPackagePathText", "Select URDF package path"))
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
								.Text(LOCTEXT("UrdfPackagePathText", "URDF Package:"))
								.Font(CreateFont(10))
							]
							+ SHorizontalBox::Slot()
							.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SNew(SEditableTextBox)
								.MinDesiredWidth(150.0f)
								.Text(this, &SAGX_ImportDialog::GetUrdfPackagePathText)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("BrowseUrdfPackageButtonText", "Browse..."))
								.ToolTipText(LOCTEXT("BrowseUrdfPackageButtonTooltip",
									"Browse to the URDF package directory. This directory corresponds to the "
									"package:// part of any filename path used in the URDF (.urdf) file"))
								.OnClicked(this, &SAGX_ImportDialog::OnBrowseUrdfPackageButtonClicked)
							]
						]	
				];
	// clang-format on
}

TSharedRef<SWidget> SAGX_ImportDialog::CreateCheckboxGui()
{
	using namespace AGX_ImportDialog_helpers;

	// clang-format off
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(5.f, 5.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
			.AutoWidth()
			[
				SNew(SCheckBox)
					.ToolTipText(LOCTEXT("IgnoreDisabledTrimeshCheckBoxTooltip",
						"Any Trimesh that has collision disabled will be ignored. "
						"Only its visual representation will be imported."))
					.OnCheckStateChanged(this, &SAGX_ImportDialog::OnIgnoreDisabledTrimeshCheckboxClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("IgnoreDisabledTrimeshText", "Ignore disabled trimeshes (recommended for large models)"))
				.Font(CreateFont(10))
			]
		];
	// clang-format on
}

FReply SAGX_ImportDialog::OnBrowseFileButtonClicked()
{
	FilePath = FAGX_EditorUtilities::SelectExistingFileDialog(
		"All files", ".agx;*.urdf");
	ImportType = AGX_ImportDialog_helpers::GetFrom(FilePath);

	RefreshGui();

	if (ImportType == EAGX_ImportType::Invalid && !FilePath.IsEmpty())
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

FReply SAGX_ImportDialog::OnBrowseUrdfPackageButtonClicked()
{
	const FString UrdfDir = FPaths::GetPath(FilePath);
	const FString StartDir = FPaths::DirectoryExists(UrdfDir) ? UrdfDir : FString("");
	UrdfPackagePath = FAGX_EditorUtilities::SelectExistingDirectoryDialog(
		"Select URDF package directory", StartDir, true);
	return FReply::Handled();
}

FText SAGX_ImportDialog::GetUrdfPackagePathText() const
{
	return FText::FromString(UrdfPackagePath);
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

	FAGX_ImportSettings Settings;
	Settings.FilePath = FilePath;
	Settings.IgnoreDisabledTrimeshes = IgnoreDisabledTrimesh;
	Settings.ImportType = ImportType;
	Settings.OpenBlueprintEditorAfterImport = true;
	Settings.UrdfPackagePath = UrdfPackagePath;

	AGX_ImporterToBlueprint::Import(Settings);
	return FReply::Handled();
}

void SAGX_ImportDialog::OnIgnoreDisabledTrimeshCheckboxClicked(ECheckBoxState NewCheckedState)
{
	IgnoreDisabledTrimesh = NewCheckedState == ECheckBoxState::Checked;
}

void SAGX_ImportDialog::RefreshGui()
{
	Construct(FArguments());
}

#undef LOCTEXT_NAMESPACE
