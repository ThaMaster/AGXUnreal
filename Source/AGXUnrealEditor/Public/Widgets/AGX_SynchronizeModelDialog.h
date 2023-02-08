// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Widgets/AGX_ImportDialogBase.h"

struct FAGX_SynchronizeModelSettings;

class SAGX_SynchronizeModelDialog : public SAGX_ImportDialogBase
{
public:
	void Construct(const FArguments& InArgs) override;

	TOptional<FAGX_SynchronizeModelSettings> ToSynchronizeModelSettings();

private:
	TSharedRef<SBorder> CreateSettingsGui();
	TSharedRef<SBorder> CreateSynchronizeButtonGui();

	FReply OnSynchronizeButtonClicked();
};
