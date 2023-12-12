// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Widgets/AGX_ImportDialogBase.h"

struct FAGX_ImportSettings;

class SAGX_ImportDialog : public SAGX_ImportDialogBase
{
public:
	void Construct(const FArguments& InArgs) override;

	TOptional<FAGX_ImportSettings> ToImportSettings();

private:
	TSharedRef<SBorder> CreateSettingsGui();
	TSharedRef<SBorder> CreateImportButtonGui();
	TSharedRef<SBorder> CreateURDFFileGui();
	FText GetUrdfPackagePathText() const;
	FReply OnBrowseUrdfPackageButtonClicked();
	FReply OnImportButtonClicked();
	void OnUrdfPackagePathTextCommitted(const FText& InNewText, ETextCommit::Type InCommitType);

	FString UrdfPackagePath;
};
