#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_LicenseDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_LicenseDialog){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetLicenseIdText() const;
	void OnLicenseIdTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FText GetActivationCodeText() const;
	void OnActivationCodeCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FReply OnActivateButtonClicked();

	TSharedRef<SWidget> CreateLicenseServiceGui();
	TSharedRef<SWidget> CreateLicenseInfoGui();

	FString LicenseId = "";
	FString ActivationCode = "";
};
