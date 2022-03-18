#include "Widgets/AGX_LicenseDialog.h"


// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"
#include "Widgets/ShapeWidget.h"


#define LOCTEXT_NAMESPACE "SAGX_LicenseDialog"

void SAGX_LicenseDialog::Construct(const FArguments& InArgs)
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
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseText", "LicenseText"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("ButtonText1", "Button1"))
				.OnClicked_Lambda(
				[]()
				{
					UE_LOG(LogAGX, Log, TEXT("Button1 was clicked."));
					return FReply::Handled();
				})
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("ButtonText2", "Button2"))
				.OnClicked_Lambda(
				[]()
				{
					UE_LOG(LogAGX, Log, TEXT("Button2 was clicked."));
					return FReply::Handled();
				})
			]
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
