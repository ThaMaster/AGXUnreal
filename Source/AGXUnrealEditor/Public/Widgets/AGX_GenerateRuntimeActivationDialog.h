#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_GenerateRuntimeActivationDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_GenerateRuntimeActivationDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

};
