// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_ImporterToEditor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImporterSettings.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "FileHelpers.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"

namespace AGX_ImporterToEditor_helpers
{
	void PreCreationSetup()
	{
		GEditor->SelectNone(false, false);
	}

	FString CreateBlueprintPackagePath(const FString& ModelName, bool IsBase)
	{
		const FString SubDirectory = IsBase ? "Blueprint" : "";

		const FString ImportDirPath = FString::Printf(
			TEXT("/Game/%s/%s/"), *FAGX_ImportUtilities::GetImportRootDirectoryName(), *ModelName);

		FString ParentPackagePath =
			FAGX_ImportUtilities::CreatePackagePath(ImportDirPath, SubDirectory);

		FGuid BaseNameGuid = FGuid::NewGuid();
		FString ParentAssetName = IsBase ? BaseNameGuid.ToString() : ModelName;

		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(ParentPackagePath, ParentAssetName);
		UPackage* ParentPackage = CreatePackage(*ParentPackagePath);
		FString Path = FPaths::GetPath(ParentPackage->GetName());

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName =
			IsBase ? "BP_Base_" + BaseNameGuid.ToString() : TEXT("BP_") + ModelName;
		FString BasePackagePath = UPackageTools::SanitizePackageName(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		auto PackageExists = [](const FString& PackagePath)
		{
			check(!FEditorFileUtils::IsMapPackageAsset(PackagePath));
			return FPackageName::DoesPackageExist(PackagePath) ||
				   FindObject<UPackage>(nullptr, *PackagePath) != nullptr;
		};

		/// \todo Should be possible to use one of the unique name creators here.
		int32 TryCount = 0;
		while (PackageExists(PackagePath))
		{
			++TryCount;
			PackagePath = BasePackagePath + "_" + FString::FromInt(TryCount);
		}

		return PackagePath;
	}

	UPackage* GetPackage(const FString& BlueprintPackagePath)
	{
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	UBlueprint* CreateBaseBlueprint(AActor& Template)
	{
		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Template.GetName(), true);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		static constexpr bool ReplaceInWorld = false;
		static constexpr bool KeepMobility = true;
		FKismetEditorUtilities::FCreateBlueprintFromActorParams Params;
		Params.bReplaceActor = ReplaceInWorld;
		Params.bKeepMobility = KeepMobility;
		Params.bOpenBlueprint = false;

		UBlueprint* Blueprint =
			FKismetEditorUtilities::CreateBlueprintFromActor(Package->GetName(), &Template, Params);
		check(Blueprint);

		FAGX_ObjectUtilities::SaveAsset(*Blueprint);
		return Blueprint;
	}

	UBlueprint* CreateChildBlueprint(const AActor& Template, UBlueprint& BaseBlueprint)
	{
		PreCreationSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(Template.GetName(), false);
		UPackage* Package = GetPackage(BlueprintPackagePath);
		const FString AssetName = FPaths::GetBaseFilename(Package->GetName());

		UBlueprint* BlueprintChild = FKismetEditorUtilities::CreateBlueprint(
			BaseBlueprint.GeneratedClass, Package, FName(AssetName), EBlueprintType::BPTYPE_Normal,
			UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(),
			FName("AGXUnrealImport"));

		check(BlueprintChild);
		FAGX_ObjectUtilities::SaveAsset(*BlueprintChild);
		return BlueprintChild;
	}

	bool ValidateImportResult(
		const FAGX_ImportResult& Result, const FAGX_ImporterSettings& Settings)
	{
		if (Result.Actor == nullptr)
		{
			const FString Text = FString::Printf(
				TEXT("Errors occurred during import. The file '%s' could not be imported. Log "
					 "category LogAGX in the Output Log may contain more information."),
				*Settings.FilePath);
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				Text, "Import model to Blueprint");
			return false;
		}

		if (Result.Result != EAGX_ImportResult::Success)
		{
			const FString Text = FString::Printf(
				TEXT("Some issues occurred during import and the result may not be the "
					 "expected result. Log category LogAGX in the Output Log may "
					 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
		}

		return true;
	}
}

UBlueprint* AGX_ImporterToEditor::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_ImporterToEditor_helpers;
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return nullptr;

	UBlueprint* BaseBlueprint = CreateBaseBlueprint(*Result.Actor);
	UBlueprint* ChildBlueprint = CreateChildBlueprint(*Result.Actor, *BaseBlueprint);

	if (Settings.bOpenBlueprintEditorAfterImport)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ChildBlueprint);

	return ChildBlueprint;
}
