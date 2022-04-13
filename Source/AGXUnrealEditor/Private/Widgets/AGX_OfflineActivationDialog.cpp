#include "Widgets/AGX_OfflineActivationDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "SAGX_OfflineActivationDialog"

void SAGX_OfflineActivationDialog::Construct(const FArguments& InArgs)
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
					CreateActicationRequestGui()
				]
			]	
		]
	];
	// clang-format on
}

TSharedRef<SWidget> SAGX_OfflineActivationDialog::CreateActicationRequestGui()
{
	auto CreateFont = [](int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	static const FString InfoText =
		"Placeholder text.";

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GenerateOfflineRequestText", "Generate offline activation request"))
				.Font(CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.f, 5.0f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(InfoText))
				.Font(CreateFont(10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 55.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LicenseIdText", "License id:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_OfflineActivationDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_OfflineActivationDialog::OnLicenseIdTextCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 21.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ActivationCodeText", "Activation code:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_OfflineActivationDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_OfflineActivationDialog::OnActivationCodeCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 53.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RequestOutputFilePathText", "Output file:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &SAGX_OfflineActivationDialog::GetActivationRequestPathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectButtonText", "Generate..."))
					.ToolTipText(LOCTEXT("SelectButtonTooltip",
						"Select an output file that will contain the offline activation request."))
					.OnClicked(this, &SAGX_OfflineActivationDialog::OnGenerateActivationRequestButtonClicked)
				]
			];
	// clang-format on
}

FText SAGX_OfflineActivationDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseId);
}

void SAGX_OfflineActivationDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseId = NewText.ToString();
}

FText SAGX_OfflineActivationDialog::GetActivationCodeText() const
{
	return FText::FromString(ActivationCode);
}

void SAGX_OfflineActivationDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	ActivationCode = NewText.ToString();
}

FReply SAGX_OfflineActivationDialog::OnGenerateActivationRequestButtonClicked()
{
	if (LicenseId.IsEmpty() || ActivationCode.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License Id or Activation code was empty. Please enter a License Id and Activation "
			"code.");
		return FReply::Handled();
	}

	if (!ContainsOnlyIntegers(LicenseId))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License id may only contain integer values.");
		return FReply::Handled();
	}

	const FString Filename = FAGX_EditorUtilities::SelectNewFileDialog(
		"Select an activation request output file", ".txt", "Text file|*.txt", "ActivationRequest.txt", "");

	if (Filename.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"No output file was selected, could not generate offline activation request.");
		return FReply::Handled();
	}

	const int32 Id = FCString::Atoi(*LicenseId);
	const auto Output = FAGX_Environment::GetInstance().GenerateOfflineActivationRequest(
		Id, ActivationCode, Filename);
	if (!Output)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Could not generate an offline activation request file. "
			"The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(
		"Offline activation request saved to: " + Output.GetValue());
	return FReply::Handled();
}

FText SAGX_OfflineActivationDialog::GetActivationRequestPathText() const
{
	return FText::FromString(ActivationRequestPath);
}


#undef LOCTEXT_NAMESPACE
