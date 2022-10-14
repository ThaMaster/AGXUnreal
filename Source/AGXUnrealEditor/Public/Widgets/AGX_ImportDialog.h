// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ImportEnums.h"

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

struct FAGX_ImportSettings;

class SAGX_ImportDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_ImportDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetFilePath(const FString& InFilePath);
	void SetFileTypes(const FString& InFileTypes);
	void SetImportType(EAGX_ImportType InImportType);

	void RefreshGui();

	TOptional<FAGX_ImportSettings> ToImportSettings();

private:
	TSharedRef<SWidget> CreateBrowseFileGui();
	TSharedRef<SBorder> CreateSettingsGui();
	TSharedRef<SBorder> CreateImportAGXFileGui();
	TSharedRef<SBorder> CreateImportURDFFileGui();	
	TSharedRef<SBorder> CreateImportButtonGui();
	TSharedRef<SWidget> CreateCheckboxGui();	

	FReply OnBrowseFileButtonClicked();
	FText GetFilePathText() const;

	FReply OnBrowseUrdfPackageButtonClicked();
	FText GetUrdfPackagePathText() const;

	FReply OnImportButtonClicked();

	void OnIgnoreDisabledTrimeshCheckboxClicked(ECheckBoxState NewCheckedState);

	FString FileTypes = ".agx;*.urdf";
	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;
	FString FilePath;
	FString UrdfPackagePath;
	bool bIgnoreDisabledTrimesh = false;
	bool bUserHasPressedImport = false;
};
