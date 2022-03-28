#include "Widgets/AGX_GenerateRuntimeActivationDialog.h"

#define LOCTEXT_NAMESPACE "SAGX_GenerateRuntimeActivationDialog"

void SAGX_GenerateRuntimeActivationDialog::Construct(const FArguments& InArgs)
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
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				//.Content()
				//[
					// Todo add content
				//]
			]	
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
