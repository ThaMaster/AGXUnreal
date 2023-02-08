// Copyright 2023, Algoryx Simulation AB.

#include "Widgets/AGX_SynchronizeModelDialog.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "SAGX_SynchronizeModelDialog"

void SAGX_SynchronizeModelDialog::Construct(const FArguments& InArgs)
{
	FileTypes = ".agx";
	ImportType = EAGX_ImportType::Agx;

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
				CreateImportAGXFileGui()
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
				CreateSynchronizeButtonGui()
			]
		]
	];
	// clang-format on
}

TOptional<FAGX_SynchronizeModelSettings> SAGX_SynchronizeModelDialog::ToSynchronizeModelSettings()
{
	if (!bUserHasPressedImport)
	{
		// The Window containing this Widget was closed, the user never pressed Synchronize.
		return {};
	}

	if (FilePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"A file must be selected before synchronizing the model.");
		return {};
	}

	FAGX_SynchronizeModelSettings Settings;
	Settings.FilePath = FilePath;
	Settings.bIgnoreDisabledTrimeshes = bIgnoreDisabledTrimesh;
	Settings.bOpenBlueprintEditorAfterImport = true;
	return Settings;
}

TSharedRef<SBorder> SAGX_SynchronizeModelDialog::CreateSettingsGui()
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
				CreateCheckboxGui()
			]			
		];
	// clang-format on
}

TSharedRef<SBorder> SAGX_SynchronizeModelDialog::CreateSynchronizeButtonGui()
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
						.Text(LOCTEXT("SynchronizeButtonText", "Synchronize Model"))
						.ToolTipText(LOCTEXT("SynchronizeButtonTooltip",
							"Synchronizes the model against the source file of the original "
							"import and updates the Components and Assets to match said source file."))
						.OnClicked(this, &SAGX_SynchronizeModelDialog::OnSynchronizeButtonClicked)
					]
				];

	// clang-format on
}

FReply SAGX_SynchronizeModelDialog::OnSynchronizeButtonClicked()
{
	bUserHasPressedImport = true;

	// We are done, close the Window containing this Widget. The user of this Widget should get
	// the user's input via the ToImportSettings function when the Window has closed.
	TSharedRef<SWindow> ParentWindow =
		FSlateApplication::Get().FindWidgetWindow(AsShared()).ToSharedRef();
	FSlateApplication::Get().RequestDestroyWindow(ParentWindow);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
