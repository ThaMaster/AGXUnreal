// Copyright 2024, Algoryx Simulation AB.

#include "Brick/AGX_BrickAssetCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Brick/AGX_BrickAsset.h"
#include "Brick/Brick.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_BrickAssetCustomization"

TSharedRef<IDetailCustomization> FAGX_BrickAssetCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_BrickAssetCustomization);
}

void FAGX_BrickAssetCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	const UAGX_BrickAsset* BrickAsset =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_BrickAsset>(InDetailBuilder);
	if (BrickAsset == nullptr)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Experiments");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
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
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("Import", "Import"))
						.ToolTipText(LOCTEXT("ImportTooltip",
							"Import using hard-coded path."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnImportButtonClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("Experiment", "Experiment"))
						.ToolTipText(LOCTEXT("ExperimentTooltip",
							"Experiment."))
						.OnClicked(this, &FAGX_BrickAssetCustomization::OnExperimentButtonClicked)
					]
				]
			]
		]
	];
	// clang-format on
}

FReply FAGX_BrickAssetCustomization::OnImportButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	FBrick::Test();
	return FReply::Handled();
}

namespace
{
	UPackage* CreateUPackage(const FString& BlueprintPackagePath)
	{
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
		Package->FullyLoad();
		return Package;
	}

	FString CreatePackagePath(const FString& SubDir)
	{
		const FString BasePath = FString(TEXT("/Game/"));
		return FAGX_ImportUtilities::CreatePackagePath(BasePath, SubDir);
	}

	UBlueprint* CreateBlueprint(const FString& Name)
	{
		GEditor->SelectNone(false, false);
		UPackage* Package = CreateUPackage("Blueprint");
		
	}
}

FReply FAGX_BrickAssetCustomization::OnExperimentButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	UE_LOG(LogTemp, Warning, TEXT("Experiment!"));
	UBlueprint* BaseBlueprint = ::CreateBlueprint("BaseBlueprint");


	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
