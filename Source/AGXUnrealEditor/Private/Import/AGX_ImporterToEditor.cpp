// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_ImporterToEditor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AGXToUeContext.h"
#include "AGX_LogCategory.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImporterSettings.h"
#include "Import/AGX_SCSNodeCollection.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_ImporterUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/SCS_Node.h"
#include "FileHelpers.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"

namespace AGX_ImporterToEditor_helpers
{
	void PreCreateBlueprintSetup()
	{
		GEditor->SelectNone(false, false);
	}

	void PreReimportSetup()
	{
		// During Model Synchronization, old assets are deleted and references to these assets are
		// automatically cleared. Having the Blueprint Editor opened while doing this causes
		// crashing during this process and the exact reason why is not clear. So we solve this
		// by closing all asset editors here first.
		if (GEditor != nullptr)
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
	}

	FString MakeModelName(FString SourceFilename)
	{
		return FAGX_EditorUtilities::SanitizeName(
			SourceFilename, FAGX_ImportUtilities::GetImportRootDirectoryName());
	}

	FString MakeRootDirectoryPath(const FString ModelName)
	{
		const FString ImportDirPath =
			FString::Printf(TEXT("/Game/%s/"), *FAGX_ImportUtilities::GetImportRootDirectoryName());
		FString BasePath = FAGX_ImportUtilities::CreatePackagePath(ImportDirPath, ModelName, false);

		auto PackageExists = [&](const FString& DirPath)
		{
			FString DiskPath = FPackageName::LongPackageNameToFilename(DirPath);
			return FPackageName::DoesPackageExist(DirPath) ||
				   FindObject<UPackage>(nullptr, *DirPath) != nullptr ||
				   FPaths::DirectoryExists(DiskPath) || FPaths::FileExists(DiskPath);
		};

		int32 TryCount = 0;
		FString DirectoryPath = BasePath;
		FString DirectoryName = ModelName;
		while (PackageExists(DirectoryPath))
		{
			++TryCount;
			DirectoryPath = BasePath + TEXT("_") + FString::FromInt(TryCount);
			DirectoryName = ModelName + TEXT("_") + FString::FromInt(TryCount);
		}
		UE_LOG(LogAGX, Display, TEXT("Importing model '%s' to '%s'."), *ModelName, *DirectoryPath);
		return DirectoryPath;
	}

	FString CreateBlueprintPackagePath(
		const FString& RootDir, const FString& ModelName, bool IsBase)
	{
		const FString SubDirectory = IsBase ? "Blueprint" : "";
		FString ParentPackagePath = FAGX_ImportUtilities::CreatePackagePath(RootDir, SubDirectory);
		UPackage* ParentPackage = CreatePackage(*ParentPackagePath);
		const FString Path = FPaths::GetPath(ParentPackage->GetName());

		// Create a known unique name for the Blueprint package, but don't create the actual
		// package yet.
		const FString BlueprintName =
			IsBase ? "BP_Base_" + FGuid::NewGuid().ToString() : TEXT("BP_") + ModelName;
		FString BasePackagePath = UPackageTools::SanitizePackageName(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		return PackagePath;
	}

	UPackage* GetPackage(const FString& BlueprintPackagePath)
	{
		UPackage* Package = CreatePackage(*BlueprintPackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	UBlueprint* CreateBaseBlueprint(
		const FString& RootDir, const FString& ModelName, AActor& Template)
	{
		PreCreateBlueprintSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(RootDir, ModelName, true);
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

	UBlueprint* CreateChildBlueprint(
		const FString& RootDir, const FString& ModelName, UBlueprint& BaseBlueprint)
	{
		PreCreateBlueprintSetup();
		FString BlueprintPackagePath = CreateBlueprintPackagePath(RootDir, ModelName, false);
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

	void UpdateBlueprint(UBlueprint& Blueprint, const FAGX_AGXToUeContext& Context)
	{
		FAGX_SCSNodeCollection Nodes(Blueprint);

		if (Context.RigidBodies != nullptr)
		{
			for (const auto& [Guid, RigidBody] : *Context.RigidBodies)
			{
				USCS_Node* N = Nodes.RigidBodies.FindRef(Guid);
				if (N == nullptr)
					continue; // Todo: create new.

				FAGX_ImporterUtilities::CopyProperties(
					*RigidBody, *Cast<UAGX_RigidBodyComponent>(N->ComponentTemplate));
			}
		}
	}

	void WriteAssetToDisk(const FString& RootDir, const FString& AssetType, UObject& Object)
	{
		FString AssetName = FAGX_ImportUtilities::CreateAssetName(Object.GetName(), "", AssetType);
		FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(RootDir, AssetType);
		FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
		UPackage* Package = CreatePackage(*PackagePath);
		Object.Rename(*Object.GetName(), Package);
		Package->MarkPackageDirty();
		Object.SetFlags(RF_Public | RF_Standalone);
		FAGX_ObjectUtilities::SaveAsset(Object);
	}

	void WriteAssetsToDisk(const FString& RootDir, const FAGX_AGXToUeContext* Context)
	{
		if (Context == nullptr)
			return;

		if (Context->MSThresholds != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();
			for (const auto& [Guid, MST] : *Context->MSThresholds)
			{
				if (auto SCMST = Cast<UAGX_ShapeContactMergeSplitThresholds>(MST))
					WriteAssetToDisk(RootDir, AssetType, *SCMST);
			}
		}
	}
}

UBlueprint* FAGX_ImporterToEditor::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_ImporterToEditor_helpers;
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return nullptr;

	ModelName = MakeModelName(Result.Actor->GetName());
	RootDirectory = MakeRootDirectoryPath(ModelName);

	WriteAssetsToDisk(RootDirectory, Result.Context);

	UBlueprint* BaseBlueprint = CreateBaseBlueprint(RootDirectory, ModelName, *Result.Actor);
	UBlueprint* ChildBlueprint = CreateChildBlueprint(RootDirectory, ModelName, *BaseBlueprint);

	if (Settings.bOpenBlueprintEditorAfterImport)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ChildBlueprint);

	return ChildBlueprint;
}

bool FAGX_ImporterToEditor::Reimport(
	UBlueprint& BaseBP, const FAGX_ImporterSettings& Settings, UBlueprint* OpenBlueprint)
{
	using namespace AGX_ImporterToEditor_helpers;
	PreReimportSetup();
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return false;

	UpdateBlueprint(BaseBP, Importer.GetContext());

	if (Settings.bOpenBlueprintEditorAfterImport && OpenBlueprint != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(OpenBlueprint);

	return true;
}
