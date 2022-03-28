#include "Widgets/AGX_LicenseDialog.h"


// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"
#include "Widgets/ShapeWidget.h"
#include "Widgets/Layout/SScrollBox.h"

// Standard library includes.
#include <algorithm>

#define LOCTEXT_NAMESPACE "SAGX_LicenseDialog"

namespace AGX_LicenseDialog_helpers
{
	bool containsOnlyIntegers(const FString& str)
	{
		return std::all_of(
			str.begin(), str.end(), [](TCHAR C) { return TChar<TCHAR>::IsDigit(C); });
	}

	FSlateFontInfo CreateFont(int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	FString CreateLicenseInfo(const FString& LicenseStatus)
	{
		FString Info("");

		if (!LicenseStatus.IsEmpty())
		{
			Info.Append("Status: " + LicenseStatus + "\n");
		}

		const TArray<FString> Keys {"User", "Contact", "EndDate"};
		for (const auto& Key : Keys)
		{
			if (const auto Value = FAGX_Environment::GetInstance().GetAgxDynamicsLicenseValue(Key))
			{
				Info.Append(Key + ": " + Value.GetValue() + "\n");
			}
		}

		return Info;
	}
}

void SAGX_LicenseDialog::Construct(const FArguments& InArgs)
{
	UpdateLicenseDialogData();

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
					CreateLicenseInfoGui()
				]
			]
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
					CreateLicenseServiceGui()
				]
			]	
		]
	];
	// clang-format on
}

void SAGX_LicenseDialog::UpdateLicenseDialogData()
{
	FString LicenseStatus;
	const bool LicenseValid =
		FAGX_Environment::GetInstance().EnsureAgxDynamicsLicenseValid(&LicenseStatus);

	if (LicenseValid)
	{
		LicenseData.LicenseValidityTextColor = FSlateColor(FLinearColor::Green);
		LicenseData.LicenseValidity = "License: Valid";
	}
	else
	{
		LicenseData.LicenseValidityTextColor = FSlateColor(FLinearColor::Red);
		LicenseData.LicenseValidity = "License: Invalid";
	}

	LicenseData.LicenseInfo = AGX_LicenseDialog_helpers::CreateLicenseInfo(LicenseStatus);

	LicenseData.EnabledModules.Empty();
	for (const FString& Module : FAGX_Environment::GetInstance().GetAgxDynamicsEnabledModules())
	{
		if (Module.Equals("AgX"))
		{
			continue;
		}

		const FString ModuleFormatted = [&Module]() 
		{ 
			FString S = Module;
			return S.Replace(TEXT("AgX-"), TEXT("AGX-"), ESearchCase::CaseSensitive);
		}();

		LicenseData.EnabledModules.Add(MakeShareable(new FString(ModuleFormatted)));
	}
}

FText SAGX_LicenseDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseData.LicenseId);
}

void SAGX_LicenseDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseData.LicenseId = NewText.ToString();
}

FText SAGX_LicenseDialog::GetActivationCodeText() const
{
	return FText::FromString(LicenseData.ActivationCode);
}

void SAGX_LicenseDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseData.ActivationCode = NewText.ToString();
}

FReply SAGX_LicenseDialog::OnActivateButtonClicked()
{
	using namespace AGX_LicenseDialog_helpers;
	if (LicenseData.LicenseId.IsEmpty() || LicenseData.ActivationCode.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License Id or Activation code was empty. Please enter a License Id and Activation "
			"code.");
		return FReply::Handled();
	}

	if (!containsOnlyIntegers(LicenseData.LicenseId))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License id may only contain integer values.");
		return FReply::Handled();
	}

	const int32 Id = FCString::Atoi(*LicenseData.LicenseId);
	const bool Result = FAGX_Environment::GetInstance().ActivateAgxDynamicsServiceLicense(
		Id, LicenseData.ActivationCode);
	UpdateLicenseDialogData();

	// @todo Figure out how to update the EnabledModules combobox without calling Construct here.
	// All other text fields are updated in the the GUI as expected thanks to having their values
	// bound to a function. The ComboBox OptionSouce does not seem to behave that way.
	Construct(FArguments());

	if (!Result)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License activation was unsuccessful. The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog("License activation was successful.");
	return FReply::Handled();
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseServiceGui()
{
	using namespace AGX_LicenseDialog_helpers;

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
					.ToolTipText(LOCTEXT("ActivateButtonTooltip",
						"Activate AGX Dynamics service license given License id and Activation code."))
					.OnClicked(this, &SAGX_LicenseDialog::OnActivateButtonClicked)
				]
			];
	// clang-format on
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseInfoGui()
{
	using namespace AGX_LicenseDialog_helpers;

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseInfoText", "License information"))
				.Font(CreateFont(16))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateLicenseValidityTextBlock()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SAGX_LicenseDialog::GetLicenseInfoText)
					.Font(CreateFont(10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SComboBox<TSharedPtr<FString>>)							
					.OptionsSource(&LicenseData.EnabledModules)
					.OnGenerateWidget_Lambda([=](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock)
							.Text(FText::FromString(*Item));
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(FText::FromString("<Enabled Modules>"))
					]
				]
			];
	// clang-format on
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseValidityTextBlock() const
{
	using namespace AGX_LicenseDialog_helpers;

	return SNew(STextBlock)
		.Text(this, &SAGX_LicenseDialog::GetLicenseValidityText)
		.Font(CreateFont(10))
		.ColorAndOpacity(this, &SAGX_LicenseDialog::GetLicenseValidityTextColor);
}

FText SAGX_LicenseDialog::GetLicenseValidityText() const
{
	return FText::FromString(LicenseData.LicenseValidity);
}

FSlateColor SAGX_LicenseDialog::GetLicenseValidityTextColor() const
{
	return LicenseData.LicenseValidityTextColor;
}

FText SAGX_LicenseDialog::GetLicenseInfoText() const
{
	return FText::FromString(LicenseData.LicenseInfo);
}

#undef LOCTEXT_NAMESPACE
