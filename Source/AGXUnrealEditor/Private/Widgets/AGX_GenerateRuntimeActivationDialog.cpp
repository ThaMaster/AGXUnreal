#include "Widgets/AGX_GenerateRuntimeActivationDialog.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"

#define LOCTEXT_NAMESPACE "SAGX_GenerateRuntimeActivationDialog"

void SAGX_GenerateRuntimeActivationDialog::Construct(const FArguments& InArgs)
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
					CreateUserInputGui()
				]
			]	
		]
	];
	// clang-format on
}

TSharedRef<SWidget> SAGX_GenerateRuntimeActivationDialog::CreateUserInputGui()
{
	auto CreateFont = [](int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GenerateRuntimeActivationText", "Generate runtime activation"))
				.Font(CreateFont(16))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 45.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LicenseIdText", "License id:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_GenerateRuntimeActivationDialog::OnLicenseIdTextCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 10.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ActivationCodeText", "Activation code:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_GenerateRuntimeActivationDialog::OnActivationCodeCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 22.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ReferenceFilePathText", "Reference file:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetReferenceFilePathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseButtonText", "Browse..."))
					.ToolTipText(LOCTEXT("BrowseButtonTooltip",
						"Browse a unique reference file within the built application that a runtime activation "
						"will be generated for."))
					.OnClicked(this, &SAGX_GenerateRuntimeActivationDialog::OnBrowseReferenceFileButtonClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("GenerateButtonText", "  Generate  "))
					.ToolTipText(LOCTEXT("GenerateButtonTooltip",
						"Generate an AGX Dynamics runtime activation for the specified application "
						"given License id and Activation code."))
					.OnClicked(this, &SAGX_GenerateRuntimeActivationDialog::OnActivateButtonClicked)
				]
			];
	// clang-format on
}

FText SAGX_GenerateRuntimeActivationDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseId);
}

void SAGX_GenerateRuntimeActivationDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseId = NewText.ToString();
}

FText SAGX_GenerateRuntimeActivationDialog::GetActivationCodeText() const
{
	return FText::FromString(ActivationCode);
}

void SAGX_GenerateRuntimeActivationDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	ActivationCode = NewText.ToString();
}

FReply SAGX_GenerateRuntimeActivationDialog::OnActivateButtonClicked()
{
	return FReply::Handled();
}

FText SAGX_GenerateRuntimeActivationDialog::GetReferenceFilePathText() const
{
	return FText::FromString(ReferenceFilePath);
}

FReply SAGX_GenerateRuntimeActivationDialog::OnBrowseReferenceFileButtonClicked()
{
	ReferenceFilePath = FAGX_EditorUtilities::SelectExistingFileDialog(
		"Select reference file in built application", "");
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
