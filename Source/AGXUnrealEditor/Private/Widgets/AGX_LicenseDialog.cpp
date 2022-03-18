#include "Widgets/AGX_LicenseDialog.h"


// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"
#include "Widgets/ShapeWidget.h"
#include "Widgets/Layout/SScrollBox.h"


#define LOCTEXT_NAMESPACE "SAGX_LicenseDialog"


void SAGX_LicenseDialog::Construct(const FArguments& InArgs)
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
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateLicenseServiceGui()
				]
			]			
		]
	];
	// clang-format on
}

FText SAGX_LicenseDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseId);
}

void SAGX_LicenseDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseId = NewText.ToString();
}

FText SAGX_LicenseDialog::GetActivationCodeText() const
{
	return FText::FromString(ActivationCode);
}

void SAGX_LicenseDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	ActivationCode = NewText.ToString();
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseServiceGui()
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
				.Text(LOCTEXT("LicenseActivationServiceText", "License activation service"))
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
					.Text(this, &SAGX_LicenseDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_LicenseDialog::OnLicenseIdTextCommitted)
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
					.Text(this, &SAGX_LicenseDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_LicenseDialog::OnActivationCodeCommitted)
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
					.Text(LOCTEXT("ActivateButtonText", "  Activate  "))
					.OnClicked_Lambda(
					[]()
					{
						UE_LOG(LogAGX, Log, TEXT("Activate was clicked."));
						return FReply::Handled();
					})
				]
			];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
