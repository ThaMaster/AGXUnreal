#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_LicenseDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_LicenseDialog){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
