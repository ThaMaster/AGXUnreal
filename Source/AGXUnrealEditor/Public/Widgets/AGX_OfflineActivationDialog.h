#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_OfflineActivationDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_OfflineActivationDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateActicationRequestGui();

	FText GetLicenseIdText() const;
	void OnLicenseIdTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FText GetActivationCodeText() const;
	void OnActivationCodeCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FReply OnGenerateActivationRequestButtonClicked();
	FText GetActivationRequestPathText() const;

	FString LicenseId;
	FString ActivationCode;
	FString ActivationRequestPath;
	FString ActivationResponsePath;
};
