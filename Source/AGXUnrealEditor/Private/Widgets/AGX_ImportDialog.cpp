// Copyright 2023, Algoryx Simulation AB.

#include "Widgets/AGX_ImportDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImportSettings.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"


#define LOCTEXT_NAMESPACE "SAGX_ImportDialog"

namespace AGX_ImportDialog_helpers
{
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

void SAGX_ImportDialog::Construct(const FArguments& InArgs)
{
	FileTypes = ".agx;*.urdf";

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
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
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
				CreateAGXFileGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 0.0f))
			.AutoHeight()
			[
				CreateURDFFileGui()
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

TOptional<FAGX_ImportSettings> SAGX_ImportDialog::ToImportSettings()
{
	if (!bUserHasPressedImportOrSynchronize)
	{
		// The Window containing this Widget was closed, the user never pressed Import.
		return {};
	}

	if (FilePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"A file must be selected before importing.");
		return {};
	}

	FAGX_ImportSettings Settings;
	Settings.FilePath = FilePath;
	Settings.bIgnoreDisabledTrimeshes = bIgnoreDisabledTrimesh;
	Settings.ImportType = ImportType;
	Settings.bOpenBlueprintEditorAfterImport = true;
	Settings.UrdfPackagePath = UrdfPackagePath;
	return Settings;
}

TSharedRef<SBorder> SAGX_ImportDialog::CreateSettingsGui()
{
	if (ImportType == EAGX_ImportType::Invalid)
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)	
			+ SVerticalBox::Slot()
			.Padding(FMargin(10.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SettingsText", "Settings"))
				.Font(FAGX_SlateUtilities::CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				CreateIgnoreDisabledTrimeshGui()
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
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
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

TSharedRef<SBorder> SAGX_ImportDialog::CreateURDFFileGui()
{
	if (ImportType != EAGX_ImportType::Urdf)
	{
		return MakeShared<SBorder>();
	}

	if (!AGX_ImportDialog_helpers::UrdfHasFilenameAttribute(FilePath))
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(FMargin(10.0f, 10.0f, 10.f, 10.f))
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("BrowseUrdfPackagePathText", "Select URDF package path"))
							.Font(FAGX_SlateUtilities::CreateFont(12))
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
								.Font(FAGX_SlateUtilities::CreateFont(10))
							]
							+ SHorizontalBox::Slot()
							.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SNew(SEditableTextBox)
								.MinDesiredWidth(150.0f)
								.Text(this, &SAGX_ImportDialog::GetUrdfPackagePathText)
								.OnTextCommitted(this, &SAGX_ImportDialog::OnUrdfPackagePathTextCommitted)
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

FText SAGX_ImportDialog::GetUrdfPackagePathText() const
{
	return FText::FromString(UrdfPackagePath);
}

FReply SAGX_ImportDialog::OnImportButtonClicked()
{
	bUserHasPressedImportOrSynchronize = true;

	// We are done, close the Window containing this Widget. The user of this Widget should get
	// the user's input via the ToImportSettings function when the Window has closed.
	TSharedRef<SWindow> ParentWindow =
		FSlateApplication::Get().FindWidgetWindow(AsShared()).ToSharedRef();
	FSlateApplication::Get().RequestDestroyWindow(ParentWindow);

	return FReply::Handled();
}

FReply SAGX_ImportDialog::OnBrowseUrdfPackageButtonClicked()
{
	const FString UrdfDir = FPaths::GetPath(FilePath);
	const FString StartDir = FPaths::DirectoryExists(UrdfDir) ? UrdfDir : FString("");
	UrdfPackagePath = FAGX_EditorUtilities::SelectExistingDirectoryDialog(
		"Select URDF package directory", StartDir, true);
	return FReply::Handled();
}

void SAGX_ImportDialog::OnUrdfPackagePathTextCommitted(
	const FText& InNewText, ETextCommit::Type InCommitType)
{
	UrdfPackagePath = InNewText.ToString();
}

#undef LOCTEXT_NAMESPACE
