// Copyright 2022, Algoryx Simulation AB.

#include "AGX_ReImportComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ReImportComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_ReImportComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ReImportComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ReImportComponentCustomization);
}

void FAGX_ReImportComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_ReImportComponent* ReImportComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ReImportComponent>(
			InDetailBuilder);
	if (!ReImportComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("AGX Re-Import");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ReImportButtonText", "Re-import"))
			.ToolTipText(LOCTEXT(
				"ReImportButtonTooltip",
				"Re-imports the model and updates the Components to match the source file."))
			.OnClicked(this, &FAGX_ReImportComponentCustomization::OnReImportButtonClicked)
		]
	];
	// clang-format on

	InDetailBuilder.HideCategory(FName("AGX Re-import Info"));
	InDetailBuilder.HideCategory(FName("Variable"));
	InDetailBuilder.HideCategory(FName("Sockets"));
	InDetailBuilder.HideCategory(FName("Tags"));
	InDetailBuilder.HideCategory(FName("ComponentTick"));
	InDetailBuilder.HideCategory(FName("ComponentReplication"));
	InDetailBuilder.HideCategory(FName("Activation"));
	InDetailBuilder.HideCategory(FName("Cooking"));
	InDetailBuilder.HideCategory(FName("Events"));
	InDetailBuilder.HideCategory(FName("AssetUserData"));
	InDetailBuilder.HideCategory(FName("Collision"));
}

namespace AGX_ReImportComponentCustomization_helpers
{
	UBlueprint* GetOutermostParent(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_ReImportComponent* ReImportComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ReImportComponent>(
				DetailBuilder);
		if (ReImportComponent == nullptr)
		{
			nullptr;
		}

		if (!ReImportComponent->IsInBlueprint())
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Re-import is only supported when in a Blueprint.");
			return nullptr;
		}

		// Get outermost parent Blueprint, which is the "original" and should match the result of
		// the previous import.
		UBlueprint* OuterMostParent = nullptr;
		if (auto Bp = Cast<UBlueprintGeneratedClass>(ReImportComponent->GetOuter()))
		{
			if (Bp->SimpleConstructionScript != nullptr)
			{
				OuterMostParent = FAGX_BlueprintUtilities::GetOutermostParent(
					Bp->SimpleConstructionScript->GetBlueprint());
			}
		}

		if (OuterMostParent == nullptr)
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Could not get the original parent Blueprint. Re-import will not be performed.");
			return nullptr;
		}

		return OuterMostParent;
	}

	FString GetSourceFilePath(UBlueprint& BaseBP)
	{
		UAGX_ReImportComponent* ReImportComponent =
			FAGX_BlueprintUtilities::GetFirstComponentOfType<UAGX_ReImportComponent>(&BaseBP);

		// Attempt using the file path stored in the ReImportComponent.
		FString FilePath = ReImportComponent != nullptr ? ReImportComponent->FilePath : "";
		if (!FPaths::FileExists(FilePath))
		{
			return "";
		}

		return FilePath;
	}
}

FReply FAGX_ReImportComponentCustomization::OnReImportButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	using namespace AGX_ReImportComponentCustomization_helpers;
	UBlueprint* OutermostParent = GetOutermostParent(*DetailBuilder);
	if (OutermostParent == nullptr)
	{
		// Logging done in GetOutermostParent.
		return FReply::Handled();
	}

	// Open up the import settings Window to get user import settings.
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(NSLOCTEXT("AGX", "AGXUnrealReImport", "Re-import AGX Dynamics archive"));

	const FString FilePath = GetSourceFilePath(*OutermostParent);
	TSharedRef<SAGX_ImportDialog> ImportDialog = SNew(SAGX_ImportDialog);
	ImportDialog->SetFilePath(FilePath);
	ImportDialog->SetImportType(EAGX_ImportType::Agx);
	ImportDialog->SetFileTypes(".agx");
	ImportDialog->RefreshGui();
	Window->SetContent(ImportDialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);
	if (auto ImportSettings = ImportDialog->ToImportSettings())
	{
		AGX_ImporterToBlueprint::ReImport(*OutermostParent, *ImportSettings);
	}

	// Logging done in ReImport.
	return FReply::Handled(); 
}

#undef LOCTEXT_NAMESPACE
