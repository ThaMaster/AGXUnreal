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

void FAGX_ReImportComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
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
				"Re-imports the model and updates the Components and Assets to match the source file."))
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
	UBlueprint* GetBlueprint(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_ReImportComponent* ReImportComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ReImportComponent>(
				DetailBuilder);
		if (ReImportComponent == nullptr)
		{
			return nullptr;
		}

		if (!ReImportComponent->IsInBlueprint())
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Re-import is only supported when in a Blueprint.");
			return nullptr;
		}

		if (auto Bp = Cast<UBlueprintGeneratedClass>(ReImportComponent->GetOuter()))
		{
			if (Bp->SimpleConstructionScript != nullptr)
			{
				return Bp->SimpleConstructionScript->GetBlueprint();
			}
		}

		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Unable to get the Blueprint from the Re-import Component. Re-import will not be "
			"possible.");
		return nullptr;
	}

	UBlueprint* GetOutermostParent(IDetailLayoutBuilder& DetailBuilder, UBlueprint& Blueprint)
	{
		UBlueprint* OuterMostParent = FAGX_BlueprintUtilities::GetOutermostParent(&Blueprint);

		if (OuterMostParent == nullptr)
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Could not get the original parent Blueprint. Re-import will not be performed.");
			return nullptr;
		}

		return OuterMostParent;
	}
}

FReply FAGX_ReImportComponentCustomization::OnReImportButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	using namespace AGX_ReImportComponentCustomization_helpers;
	UBlueprint* Blueprint = GetBlueprint(*DetailBuilder);
	if (Blueprint == nullptr)
	{
		// Logging done in GetBlueprint.
		return FReply::Handled();
	}

	UBlueprint* OutermostParent = GetOutermostParent(*DetailBuilder, *Blueprint);
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

	UAGX_ReImportComponent* ReImportComponent =
		FAGX_BlueprintUtilities::GetFirstComponentOfType<UAGX_ReImportComponent>(OutermostParent);
	const FString FilePath = ReImportComponent != nullptr ? ReImportComponent->FilePath : "";
	const bool IgnoreDisabledTrimeshes =
		ReImportComponent != nullptr ? ReImportComponent->bIgnoreDisabledTrimeshes : false;

	TSharedRef<SAGX_ImportDialog> ImportDialog = SNew(SAGX_ImportDialog);
	ImportDialog->SetFilePath(FilePath);
	ImportDialog->SetIgnoreDisabledTrimeshes(IgnoreDisabledTrimeshes);
	ImportDialog->SetImportType(EAGX_ImportType::Agx);
	ImportDialog->SetFileTypes(".agx");
	ImportDialog->RefreshGui();
	Window->SetContent(ImportDialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);

	if (auto ImportSettings = ImportDialog->ToImportSettings())
	{
		const static FString Info = "Re-import may remove or overwrite existing data.\nContinue?";
		if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Info)) !=
			EAppReturnType::Yes)
		{
			return FReply::Handled();
		}

		if (AGX_ImporterToBlueprint::ReImport(*OutermostParent, *ImportSettings))
		{
			FAGX_EditorUtilities::SaveAndCompile(*Blueprint);
		}
	}

	// Any logging is done in ReImport and ToImportSettings.
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
