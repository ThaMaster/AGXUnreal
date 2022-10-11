// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ImportEnums.h"

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_ImportDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_ImportDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateBrowseFileGui();

	FReply OnBrowseFileButtonClicked();
	FText GetFilePathText() const;

	FReply OnImportButtonClicked();

	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;
	FString FilePath;
	FString UrdfPackagePath;
	bool SkipDisabledTrimesh = false;
};
