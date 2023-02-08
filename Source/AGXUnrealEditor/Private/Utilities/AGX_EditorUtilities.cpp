// Copyright 2023, Algoryx Simulation AB.

#include "Utilities/AGX_EditorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ImporterToBlueprint.h"
#include "AGX_LogCategory.h"
#include "AGX_ModelSourceComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/RenderDataBarrier.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "AssetDeleteModel.h"
#include "AssetToolsModule.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Editor/EditorEngine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameEngine.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/Char.h"
#include "Misc/EngineVersionComparison.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "RawMesh.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "Serialization/FindReferencersArchive.h"
#include "UObject/SavePackage.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

bool FAGX_EditorUtilities::SynchronizeModel(UBlueprint& Blueprint)
{
	UBlueprint* OuterMostParent = FAGX_BlueprintUtilities::GetOutermostParent(&Blueprint);

	if (OuterMostParent == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Could not get the original parent Blueprint. Model synchronization will not be "
			"performed.");
		return false;
	}

	// Ensure there exists a Model Source Component.
	UAGX_ModelSourceComponent* ModelSourceComponent =
		FAGX_BlueprintUtilities::GetFirstComponentOfType<UAGX_ModelSourceComponent>(
			OuterMostParent);
	if (ModelSourceComponent == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Could not find an AGX Model Source Component in the selected Blueprint. The selected "
			"Blueprint is not valid for Model Synchronization.");
		return false;
	}

	// Open up the import settings Window to get user import settings.
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(NSLOCTEXT(
				"AGX", "AGXUnrealSynchronizeModel", "Synchronize model with source file"));

	const FString FilePath = ModelSourceComponent != nullptr ? ModelSourceComponent->FilePath : "";
	const bool IgnoreDisabledTrimeshes =
		ModelSourceComponent != nullptr ? ModelSourceComponent->bIgnoreDisabledTrimeshes : false;

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
		const static FString Info =
			"Model synchronization may permanently remove or overwrite existing data.\nContinue?";
		if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Info)) !=
			EAppReturnType::Yes)
		{
			return false;
		}

		if (AGX_ImporterToBlueprint::SynchronizeModel(*OuterMostParent, *ImportSettings))
		{
			SaveAndCompile(Blueprint);
		}
	}

	return true;
}

bool FAGX_EditorUtilities::RenameAsset(
	UObject& Asset, const FString& WantedName, const FString& AssetType)
{
	if (WantedName.IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("RenameAsset called with Asset '%s' and WantedName was empty. The asset will not "
				 "be renamed."),
			*Asset.GetName());
		return false;
	}

	UPackage* Package = Asset.GetPackage();
	if (Package == nullptr || Package->GetPathName().IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"RenameAsset called with Asset '%s' without a valid Package. The asset will not be "
				"renamed."),
			*Asset.GetName());
		return false;
	}

	FString AssetNameNew = FAGX_ImportUtilities::CreateAssetName(WantedName, "", AssetType);
	if (Asset.GetName().Equals(AssetNameNew))
	{
		return true;
	}

	FString PackagePathNew = *Asset.GetPackage()->GetPathName();
	PackagePathNew.RemoveFromEnd(Asset.GetName(), ESearchCase::CaseSensitive);
	FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePathNew, AssetNameNew);

	const FString OldPath = Asset.GetPathName();

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<FAssetRenameData> AssetRenameData;
	FAssetRenameData Ard;
	Ard.Asset = &Asset;
	Ard.NewPackagePath = FPackageName::GetLongPackagePath(PackagePathNew);
	Ard.NewName = AssetNameNew;
	AssetRenameData.Add(Ard);
	AssetToolsModule.Get().RenameAssets(AssetRenameData);
	FAssetRegistryModule::AssetRenamed(&Asset, OldPath);
	Asset.MarkPackageDirty();
	Asset.GetOuter()->MarkPackageDirty();

	return true;
}

namespace AGX_EditorUtilities_helpers
{

/// @todo This has been moved to a public member function instead. Ensure they are the same and then
/// delete this.
#if 0
	/**
	 * Delete function based on BlueprintEditorPromotionTestHelper::Cleanup in
	 * BlueprintEditorTests.cpp in the engine code. It consists of four steps:
	 *  - AssetRegistry.AssetDeleted
	 *  - NullReferencesToObject
	 *  - ObjectTools::DeleteSingleObject
	 *  - Filesystem delete.
	 *
	 * The first step is straight-forward, get the asset registry and call AssetDeleted on it.
	 *
	 * The second step is implemented behind FAutomationEditorCommonUtils. I don't think we should
	 * call that directly, calling into test code from production editor code is backwards. Here I
	 * have recreated the bits of the code I think are important for our use-case as a helper
	 * function.
	 *
	 * The call to NullReferencesToObject may be expensive, so the BlueprintEditorTest first tries
	 * to call DeleteSingleObject and only if that fails does to call NullReferencesToObject.
	 *
	 * The filesystem delete deletes an entire directory. In our case I think it is enough to
	 * delete a single .uasset file.
	 */

	void NullReferencesToObject(UObject* ToDelete)
	{
		TArray<UObject*> ReplaceableObjects;
		TMap<UObject*, UObject*> ReplacementMap;
		ReplacementMap.Add(ToDelete, nullptr);
		ReplacementMap.GenerateKeyArray(ReplaceableObjects);

		// Find all the properties (and their corresponding objects) that refer to any of the
		// objects to be replaced
		TMap<UObject*, TArray<FProperty*>> ReferencingPropertiesMap;
		for (FThreadSafeObjectIterator ObjIter; ObjIter; ++ObjIter)
		{
			UObject* CurObject = *ObjIter;

			// Find the referencers of the objects to be replaced
			FFindReferencersArchive FindRefsArchive(CurObject, ReplaceableObjects);

			// Inform the object referencing any of the objects to be replaced about the properties
			// that are being forcefully changed, and store both the object doing the referencing as
			// well as the properties that were changed in a map (so that we can correctly call
			// PostEditChange later)
			TMap<UObject*, int32> CurNumReferencesMap;
			TMultiMap<UObject*, FProperty*> CurReferencingPropertiesMMap;
			if (FindRefsArchive.GetReferenceCounts(
					CurNumReferencesMap, CurReferencingPropertiesMMap) > 0)
			{
				TArray<FProperty*> CurReferencedProperties;
				CurReferencingPropertiesMMap.GenerateValueArray(CurReferencedProperties);
				ReferencingPropertiesMap.Add(CurObject, CurReferencedProperties);
				for (TArray<FProperty*>::TConstIterator RefPropIter(CurReferencedProperties);
					 RefPropIter; ++RefPropIter)
				{
					CurObject->PreEditChange(*RefPropIter);
				}
			}
		}

		// Iterate over the map of referencing objects/changed properties, forcefully replacing the
		// references and then alerting the referencing objects the change has completed via
		// PostEditChange
		int32 NumObjsReplaced = 0;
		for (TMap<UObject*, TArray<FProperty*>>::TConstIterator MapIter(ReferencingPropertiesMap);
			 MapIter; ++MapIter)
		{
			++NumObjsReplaced;

			UObject* CurReplaceObj = MapIter.Key();
			const TArray<FProperty*>& RefPropArray = MapIter.Value();
			FArchiveReplaceObjectRef<UObject> ReplaceAr(
				CurReplaceObj, ReplacementMap,
#if UE_VERSION_OLDER_THAN(5, 0, 0)
				false, true, false
#else
				EArchiveReplaceObjectFlags::IgnoreOuterRef
#endif
			);
			for (TArray<FProperty*>::TConstIterator RefPropIter(RefPropArray); RefPropIter;
				 ++RefPropIter)
			{
				FPropertyChangedEvent PropertyEvent(*RefPropIter);
				CurReplaceObj->PostEditChangeProperty(PropertyEvent);
			}

			if (!CurReplaceObj->HasAnyFlags(RF_Transient) &&
				CurReplaceObj->GetOutermost() != GetTransientPackage())
			{
				if (!CurReplaceObj->RootPackageHasAnyFlags(PKG_CompiledIn))
				{
					CurReplaceObj->MarkPackageDirty();
				}
			}
		}
	}
#endif
}

/**
 * Delete function based on BlueprintEditorPromotionTestHelper::Cleanup in
 * BlueprintEditorTests.cpp in the engine code. It consists of four steps:
 *  - AssetRegistry.AssetDeleted
 *  - NullReferencesToObject
 *  - ObjectTools::DeleteSingleObject
 *  - Filesystem delete.
 *
 * The first step is straight-forward, get the asset registry and call AssetDeleted on it.
 *
 * The second step is implemented behind FAutomationEditorCommonUtils. I don't think we should
 * call that directly, calling into test code from production editor code is backwards. Here I
 * have recreated the bits of the code I think are important for our use-case as a helper
 * function.
 *
 * The call to NullReferencesToObject may be expensive, so the BlueprintEditorTest first tries
 * to call DeleteSingleObject and only if that fails does to call NullReferencesToObject.
 *
 * The filesystem delete deletes an entire directory. In our case I think it is enough to
 * delete a single .uasset file.
 */

void FAGX_EditorUtilities::NullReferencesToObject(UObject* ToDelete)
{
	TArray<UObject*> ReplaceableObjects;
	TMap<UObject*, UObject*> ReplacementMap;
	ReplacementMap.Add(ToDelete, nullptr);
	ReplacementMap.GenerateKeyArray(ReplaceableObjects);

	// Find all the properties (and their corresponding objects) that refer to any of the
	// objects to be replaced
	TMap<UObject*, TArray<FProperty*>> ReferencingPropertiesMap;
	for (FThreadSafeObjectIterator ObjIter; ObjIter; ++ObjIter)
	{
		UObject* CurObject = *ObjIter;

		// Find the referencers of the objects to be replaced
		FFindReferencersArchive FindRefsArchive(CurObject, ReplaceableObjects);

		// Inform the object referencing any of the objects to be replaced about the properties
		// that are being forcefully changed, and store both the object doing the referencing as
		// well as the properties that were changed in a map (so that we can correctly call
		// PostEditChange later)
		TMap<UObject*, int32> CurNumReferencesMap;
		TMultiMap<UObject*, FProperty*> CurReferencingPropertiesMMap;
		if (FindRefsArchive.GetReferenceCounts(CurNumReferencesMap, CurReferencingPropertiesMMap) >
			0)
		{
			TArray<FProperty*> CurReferencedProperties;
			CurReferencingPropertiesMMap.GenerateValueArray(CurReferencedProperties);
			ReferencingPropertiesMap.Add(CurObject, CurReferencedProperties);
			for (TArray<FProperty*>::TConstIterator RefPropIter(CurReferencedProperties);
				 RefPropIter; ++RefPropIter)
			{
				CurObject->PreEditChange(*RefPropIter);
			}
		}
	}

	// Iterate over the map of referencing objects/changed properties, forcefully replacing the
	// references and then alerting the referencing objects the change has completed via
	// PostEditChange
	int32 NumObjsReplaced = 0;
	for (TMap<UObject*, TArray<FProperty*>>::TConstIterator MapIter(ReferencingPropertiesMap);
		 MapIter; ++MapIter)
	{
		++NumObjsReplaced;

		UObject* CurReplaceObj = MapIter.Key();
		const TArray<FProperty*>& RefPropArray = MapIter.Value();
		FArchiveReplaceObjectRef<UObject> ReplaceAr(
			CurReplaceObj, ReplacementMap,
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			false, true, false
#else
			EArchiveReplaceObjectFlags::IgnoreOuterRef
#endif
		);
		for (TArray<FProperty*>::TConstIterator RefPropIter(RefPropArray); RefPropIter;
			 ++RefPropIter)
		{
			FPropertyChangedEvent PropertyEvent(*RefPropIter);
			CurReplaceObj->PostEditChangeProperty(PropertyEvent);
		}

		if (!CurReplaceObj->HasAnyFlags(RF_Transient) &&
			CurReplaceObj->GetOutermost() != GetTransientPackage())
		{
			if (!CurReplaceObj->RootPackageHasAnyFlags(PKG_CompiledIn))
			{
				CurReplaceObj->MarkPackageDirty();
			}
		}
	}
}

bool FAGX_EditorUtilities::DeleteAsset(UObject& Asset)
{
	const FString AssetPath = Asset.GetPathName();
	UE_LOG(LogAGX, Warning, TEXT("FullPath='%s'"), *AssetPath);
	const FString BasePath = [&AssetPath]()
	{
		FString Result;
		AssetPath.Split(TEXT("."), &Result, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		return Result + TEXT(".uasset");
	}();
	const FString RelativeFileSystemPath = FPackageName::LongPackageNameToFilename(BasePath);
	const FString FileSystemPath = FPaths::ConvertRelativePathToFull(RelativeFileSystemPath);

	UE_LOG(LogAGX, Warning, TEXT("AssetPath='%s'"), *AssetPath);
	UE_LOG(LogAGX, Warning, TEXT("BasePath='%s'"), *BasePath);
	UE_LOG(LogAGX, Warning, TEXT("FilesystemPath='%s'"), *FileSystemPath);

	IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();

	if (GEditor != nullptr)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(&Asset);
	}

	AssetRegistry.AssetDeleted(&Asset);
	NullReferencesToObject(&Asset);
	ObjectTools::DeleteSingleObject(&Asset);

	// Try to delete the corresponding asset file. This file may not exist, in case a new asset was
	// created and immediately deleted without ever being saved.
	IFileManager::Get().Delete(*FileSystemPath, /*RequireExists*/ false);

	return true;
}

int32 FAGX_EditorUtilities::DeleteImportedAssets(const TArray<UObject*> InAssets)
{
#if 0
	UE_LOG(LogAGX, Warning, TEXT("Deleting %d assets with ObjectTools::DeleteAssets."), InAssets.Num());
	// The other implementation tries to recreate ObjectTools::(Force)DeleteObjects but without the
	// modal dialog. That causes crashes.
	//
	// This implementation tries to recreate hitting the Delete key, i.e., just call
	// ObjectTools::DeleteAssets with bShowConfirmation=true.
	TArray<FAssetData> AssetsToDelete;
	for (UObject* Asset : InAssets)
	{
		UE_LOG(LogAGX, Warning, TEXT("Asked to delete asset for object %s"), *Asset->GetPathName());
		/*
		AGX_EditorUtilities.cpp(427): warning C4996:
		'IAssetRegistry::GetAssetByObjectPath':
		Asset path FNames have been deprecated,
		use Soft Object Path instead.
		Please update your code to the new API before upgrading to the next release,
		otherwise your project will no longer compile.``
		*/
		FAssetData AssetToDelete =
			IAssetRegistry::GetChecked().GetAssetByObjectPath(FName(*Asset->GetPathName()));
		if (AssetToDelete.IsValid() && AssetToDelete.GetAsset() != nullptr)
		{
			AssetsToDelete.Add(AssetToDelete);
		}
	}
	if (AssetsToDelete.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("Did not find any assets to delete."));
		return 0;
	}
	for (FAssetData& Asset : AssetsToDelete)
	{
		UE_LOG(LogAGX, Warning, TEXT("About to delete asset %s"), *Asset.GetFullName());
	}

	for (FAssetData& AssetData : AssetsToDelete)
	{
		NullReferencesToObject(AssetData.GetAsset());
	}
	return ObjectTools::DeleteAssets(AssetsToDelete, true);

#else
	UE_LOG(LogAGX, Warning, TEXT("Deleting %d assets with FAssetDeleteModel."), InAssets.Num());

	/*
	 * This function is a subset/variant of Unreal Engine's ObjectTools::DeleteObjects. We can't use
	 * that directly because it doesn't handle references to the deleted assets properly. In
	 * particular, it always calls FAssetDeleteModel::DoDelete even though in some cases it is
	 * necessary to call DoForceDelete.
	 *
	 * Not sure how regularly ObjectTools::DeleteObjects is updated by Epic Games, and how closely
	 * we need to track that implementation over time. The initial implementation of this function
	 * is based on Unreal Engine 4.27.
	 *
	 * There is also ObjectTools::DeleteAssets which sounds like exactly what we want to do, but
	 * that is basically just a call to ObjectTools::DeleteObjects followed by
	 * CleanupAfterSuccessfulDelete. The latter is called by the former, which makes me believe that
	 * the call from ObjectTools::DeleteAssets is redundant, and the former is what we are doing a
	 * variant of here since it is incomplete.
	 */

	/// @todo We get a crash in FEditorViewportClient and don't know why.
	/// This is an attempt to work around that by cloasing all editors, which will close a bunch of
	/// viewport. Not the Level Viewport though, we can't close that one.
	///
	/// Causes a different, but very similar, crash so commenting this out again.
	// GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();

	TArray<UObject*> ObjectsToDelete = InAssets;

	// Here the engine implementation creates an FScopedBusyCursor. Should we too?

	// Here the engine implementation calls ObjectTools::AddExtraObjectsToDelete. As of Unreal
	// Engine 4.27 that only involves UWorld objects, which we don't create during model import and
	// thus don't currently need to handle here.
	//
	/// @todo Attempt to prevent crash, adding extra objects to delete.
#if 0
	ObjectTools::AddExtraObjectsToDelete(ObjectsToDelete);
#endif

	// There the engine implementation does stuff for sounds. We don't do anything with sound assets
	// so skipping that for now.

	// Here the engine implementation calls the OnAssetsCanDelete delegate. I don't know what that
	// is or what it is for, so holding off on doing that for now.
	//
	/// @todo Attempt to prevent crash, checking can delete.
	FCanDeleteAssetResult CanDeleteResult;
	FEditorDelegates::OnAssetsCanDelete.Broadcast(ObjectsToDelete, CanDeleteResult);
	if (!CanDeleteResult.Get())
	{
		UE_LOG(LogAGX, Warning, TEXT("Cannot currently delete selected objects. See log for details."));
		return 0;
	}

	// This is an experiment, the FAssetData experiment.
	TArray<FAssetData> AssetData;

	// Only packages that are fully loaded can be deleted, so ask for them to be loaded.
	//
	// Here the engine implementation calls a non-exported helper function in ObjectTools,
	// HandleFullyLoadedPackages. We can't call that from our module, so doing something similar
	// in-line instead.
	{
		TArray<UPackage*> Packages;
		for (UObject* Object : ObjectsToDelete)
		{
			Packages.AddUnique(Object->GetOutermost());

			// Part of FAssetData experiment.
			IAssetRegistry::GetChecked().GetAssetsByPath(FName(*Object->GetPathName()), AssetData);
		}
		if (!UPackageTools::HandleFullyLoadingPackages(
				Packages, LOCTEXT("DeleteImportedAssets", "Delete imported assets.")))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot delete assets because at least one asset could not be fully loaded."));
			return 0;
		}
	}

	// Part of FAssetData experiment.
	UE_LOG(LogAGX, Warning, TEXT("FAssetData found:"));
	for (FAssetData& AssetDatum : AssetData)
	{
		UE_LOG(LogAGX, Warning, TEXT(" %s"), *AssetDatum.ObjectPath.ToString());
	}

	// We cannot delete assets if the asset registry is currently busy loading assets.
	if (IAssetRegistry::GetChecked().IsLoadingAssets())
	{
		// Is there a better way to handle this case? Can we wait a bit? Is asset loading
		// asynchronous, i.e. can we spin here for a second or two?
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete assets because the asset registry is currently loading assets."));
		return 0;
	}

	// Here the engine implementation checks if the list of assets to delete include an active
	// world, including all editor world contexts and streaming levels. We currently don't create
	// any worlds during import so don't need to handle that case yet.
#if 0
	/// @todo Attempt to prevent crash, bail if attempt to delete world in use.
	{
		bool bContainsWorldInUse = [&ObjectsToDelete]()
		{
			TArray<const UWorld*> WorldsToDelete;

			for (const UObject* ObjectToDelete : ObjectsToDelete)
			{
				if (const UWorld* World = Cast<UWorld>(ObjectToDelete))
				{
					WorldsToDelete.AddUnique(World);
				}
			}

			if (WorldsToDelete.Num() == 0)
			{
				return false;
			}

			auto GetCombinedWorldNames = [](const TArray<const UWorld*>& Worlds) -> FString
			{
				return FString::JoinBy(Worlds, TEXT(", "),
					[](const UWorld* World) -> FString
					{
						return World->GetPathName();
					});
			};

			UE_LOG(LogAGX, Log, TEXT("Deleting %d worlds: %s"), WorldsToDelete.Num(), *GetCombinedWorldNames(WorldsToDelete));

			TArray<const UWorld*> ActiveWorlds;

			for (const FWorldContext& WorldContext : GEditor->GetWorldContexts())
			{
				if (const UWorld* World = WorldContext.World())
				{
					ActiveWorlds.AddUnique(World);

					for (const ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
					{
						if (StreamingLevel && StreamingLevel->GetLoadedLevel() && StreamingLevel->GetLoadedLevel()->GetOuter())
						{
							if (const UWorld* StreamingWorld = Cast<UWorld>(StreamingLevel->GetLoadedLevel()->GetOuter()))
							{
								ActiveWorlds.AddUnique(StreamingWorld);
							}
						}
					}
				}
			}

			UE_LOG(LogAGX, Log, TEXT("Currently %d active worlds: %s"), ActiveWorlds.Num(), *GetCombinedWorldNames(ActiveWorlds));

			for (const UWorld* World : WorldsToDelete)
			{
				if (ActiveWorlds.Contains(World))
				{
					return true;
				}
			}

			return false;
		}();
		if (bContainsWorldInUse)
		{
			UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot delete assets because try to delete world in use."));
			return 0;
		}
	}
#endif

	// Let everyone know that these assets are about to disappear, so they can clear any references
	// they may have to the assets.
	FEditorDelegates::OnAssetsPreDelete.Broadcast(ObjectsToDelete);

	/// @todo I don't see why I would need to do this, but it seems to fix the crash in
	/// FEditorViewportClient.
	for (UObject* Object : ObjectsToDelete)
	{
		NullReferencesToObject(Object);
	}

	// The delete model helps us find references to the deleted assets.
	/// \todo Engine code creates a shared pointer here. Is that necessary?
	FAssetDeleteModel DeleteModel(ObjectsToDelete);

	// Here the engine implementation uses GWarn to begin a slow task. The model
	// synchronize code already have a progress bar created with FScopedSlowTask, not sure how
	// those would interact.

	while (DeleteModel.GetState() != FAssetDeleteModel::Finished)
	{
		DeleteModel.Tick(0);

		// Here the engine implementation does stuff with GWarn status update and user canceling.
		// The model synchronize code already have a progress bar created with FScopedSlowTask, not
		// sure how those would interact.
	}

	if (DeleteModel.CanDelete())
	{
		DeleteModel.DoDelete();
		return DeleteModel.GetDeletedObjectCount();
	}
	else if (DeleteModel.CanForceDelete())
	{
		DeleteModel.DoForceDelete();
		return DeleteModel.GetDeletedObjectCount();
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Coult not verify safe asset deletion, old model assets remain in the Content "
				 "Browser and on drive."));
		return 0;
	}
#endif
}

std::tuple<AActor*, USceneComponent*> FAGX_EditorUtilities::CreateEmptyActor(
	const FTransform& Transform, UWorld* World)
{
	/// \todo The intention is to mimmic dragging in an "Empty Actor" from the
	/// Place mode. Investigate if we can use ActorFactoryEmptyActor instead.

	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (NewActor == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Failed to create empty actor."));
		/// \todo Do we need to destroy the Actor here?
		return {nullptr, nullptr};
	}

	/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
	/// Related to undo/redo, I think.
	USceneComponent* Root = NewObject<USceneComponent>(
		NewActor, USceneComponent::GetDefaultSceneRootVariableName() /*, RF_Transactional*/);
	NewActor->SetRootComponent(Root);
	NewActor->AddInstanceComponent(Root);
	Root->RegisterComponent();
	NewActor->SetActorTransform(Transform, false);

	return {NewActor, Root};
}

namespace
{
	template <typename TComponent>
	TComponent* CreateComponent(AActor* Owner)
	{
		UClass* Class = TComponent::StaticClass();
		TComponent* Component = NewObject<TComponent>(Owner, Class);
		if (Component == nullptr)
		{
			UE_LOG(LogAGX, Log, TEXT("Could not create component %s."), *Class->GetName());
			return nullptr;
		}
		Owner->AddInstanceComponent(Component);
		Component->RegisterComponent();
		return Component;
	}

	template <typename TShapeComponent>
	TShapeComponent* CreateShapeComponent(AActor* Owner, USceneComponent* Outer, bool bRegister)
	{
		/// \todo Is the Owner pointless here since we do `AttachToComponent`
		/// immediately afterwards?
		UClass* Class = TShapeComponent::StaticClass();
		TShapeComponent* Shape = NewObject<TShapeComponent>(
			Owner, FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));
		Owner->AddInstanceComponent(Shape);
		if (bRegister)
		{
			Shape->RegisterComponent();
		}
		if (Outer)
		{
			const bool Attached = Shape->AttachToComponent(
				Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			check(Attached);
		}
		return Shape;
	}

	template <typename T>
	static T* GetAssetByPath(const FString& AssetPath)
	{
		FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		FARFilter Filter;
		Filter.PackageNames.Add(FName(*AssetPath));
		AssetRegistryModule.Get().GetAssets(Filter, AssetData);
		T* Asset = FAssetData::GetFirstAsset<T>(AssetData);

		if (Asset == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Could not find asset with path %s."), *AssetPath);
		}

		return Asset;
	}
}

UAGX_RigidBodyComponent* FAGX_EditorUtilities::CreateRigidBody(AActor* Owner)
{
	UAGX_RigidBodyComponent* Body = ::CreateComponent<UAGX_RigidBodyComponent>(Owner);
	Body->AttachToComponent(
		Owner->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	Body->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	Body->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
#else
	Body->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Body->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
#endif
	return Body;
}

/// \todo Can the Owner parameter be removed, and instead use Outer->GetOwner()?
/// When would we want to attach the sphere to a component that is in another Actor.
UAGX_SphereShapeComponent* FAGX_EditorUtilities::CreateSphereShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_SphereShapeComponent>(Owner, Outer, true);
}

UAGX_BoxShapeComponent* FAGX_EditorUtilities::CreateBoxShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_BoxShapeComponent>(Owner, Outer, true);
}

UAGX_CylinderShapeComponent* FAGX_EditorUtilities::CreateCylinderShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CylinderShapeComponent>(Owner, Outer, true);
}

UAGX_CapsuleShapeComponent* FAGX_EditorUtilities::CreateCapsuleShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CapsuleShapeComponent>(Owner, Outer, true);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer, bool bRegister)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer, bRegister);
}

/// \todo There is probably a name sanitizer already in Unreal. Find it.
/// \todo The sanitizers are called multiple times on the same string. Find a root function, or
/// a suitable helper function, and sanitize once. Assume already sanitizied in all other helper
/// functions.

FString FAGX_EditorUtilities::SanitizeName(const FString& Name)
{
	FString Sanitized;
	Sanitized.Reserve(Name.Len());
	for (TCHAR C : Name)
	{
		/// \todo Will this accept non-english characters? Should it?
		if (TChar<TCHAR>::IsAlnum(C) || C == TCHAR('_'))
		{
			Sanitized.AppendChar(C);
		}
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const FString& Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return Fallback;
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::SanitizeName(const FString& Name, const TCHAR* Fallback)
{
	FString Sanitized = SanitizeName(Name);
	if (Sanitized.IsEmpty())
	{
		return FString(Fallback);
	}
	return Sanitized;
}

FString FAGX_EditorUtilities::CreateAssetName(
	FString SourceName, FString ActorName, FString DefaultName)
{
	SourceName = FAGX_EditorUtilities::SanitizeName(SourceName);
	if (!SourceName.IsEmpty())
	{
		return SourceName;
	}

	ActorName = FAGX_EditorUtilities::SanitizeName(ActorName);
	if (!ActorName.IsEmpty())
	{
		return ActorName;
	}

	DefaultName = FAGX_EditorUtilities::SanitizeName(DefaultName);
	if (!DefaultName.IsEmpty())
	{
		return DefaultName;
	}

	return TEXT("ImportedAGXObject");
}

void FAGX_EditorUtilities::MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName)
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackageName, AssetName, PackageName, AssetName);
}

bool FAGX_EditorUtilities::SaveStaticMeshAssetsInBulk(const TArray<UStaticMesh*>& Meshes)
{
	bool EncounteredIssue = false;
	for (auto Mesh : Meshes)
	{
		if (Mesh == nullptr)
			continue;

		FAssetRegistryModule::AssetCreated(Mesh);
		Mesh->MarkPackageDirty();
	}

	// Do the costly Build as a batch Build.
	UStaticMesh::BatchBuild(Meshes);

	for (auto Mesh : Meshes)
	{
		if (Mesh == nullptr)
			continue;

		UPackage* Package = Mesh->GetPackage();
		if (Package == nullptr || Package->GetPathName().IsEmpty())
		{
			EncounteredIssue = true;
			UE_LOG(
				LogAGX, Warning,
				TEXT("Got invalid package from asset '%s' in SaveStaticMeshAssetInBulk. The asset "
					 "will not be saved."),
				*Mesh->GetName());
		}
		// The below PostEditChange call is what normally takes a lot of time, since it internally
		// calls Build() each time. But since we have already done the Build (in batch) above, it
		// will not actually Build the asset again. So the PostEditChange call below will execute
		// really fast and we get the benefit of still getting everything else done in
		// PostEditChange.
		Mesh->PostEditChange();
		Mesh->AddToRoot();
		Package->SetDirtyFlag(true);

		// Store our new package to disk.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			Package->GetPathName(), FPackageName::GetAssetPackageExtension());
		if (PackageFilename.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unreal Engine unable to provide a package filename for package path '%s'."),
				*Package->GetPathName());
			EncounteredIssue = true;
			continue;
		}

		Package->GetMetaData();
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		bool bSaved = UPackage::SavePackage(Package, Mesh, RF_NoFlags, *PackageFilename);
#else
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		bool bSaved = UPackage::SavePackage(Package, Mesh, *PackageFilename, SaveArgs);
#endif
		if (!bSaved)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unreal Engine unable to save package '%s' to file '%s'."),
				*Package->GetPathName(), *PackageFilename);
			EncounteredIssue = true;
		}
	}

	return !EncounteredIssue;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromTrimesh(const FTrimeshShapeBarrier& Trimesh)
{
	if (Trimesh.GetNumPositions() <= 0 || Trimesh.GetNumIndices() <= 0)
	{
		// No collision mesh data available, this trimesh is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported trimesh '%s'. Cannot create "
				 "StaticMesh asset."),
			*Trimesh.GetSourceName())
		return FRawMesh();
	}

	if (Trimesh.GetNumIndices() != Trimesh.GetNumTriangles() * 3)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The trimesh '%s' does not have three vertex indices per triangle. The mesh "
				 "may be be imported incorrectly. Found %d triangles and %d indices."),
			*Trimesh.GetSourceName(), Trimesh.GetNumTriangles(), Trimesh.GetNumIndices());
	}

	// Data we have:
	//
	// The collision data consists of three arrays: positions, indices, and normals. The
	// positions is a list of vertex positions. The indices comes in triplets for each triangle
	// and the triplet defines which vertex positions form that triangle. There is one normal
	// per triangle and they are stored in the same order as the position index triplets.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3 Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |  Triangle 0   |  Triangle 1   | ... |
	//   indices:    | int, int, int | int, int, int | ... | Index into positions.
	//   normals:    |     Vec3      |      Vec3     | ... |
	//
	//
	// Data we want:
	//
	// Unreal Engine store its meshes in a similar format but with additional vertex data stored per
	// triangle. The Unreal Engine format consists of six arrays: positions, indices, tangent X,
	// tangent Y, tangent Z, and texture coordinates. Tangent Z has the same meaning as the normal
	// in the AGX Dynamics data. I don't know how to compute the X and Y tangents, but there are
	// flags to auto-compute them. See AddRawMeshToStaticMesh.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | 3x collision normal per triangle.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | (0, 0) everywhere.
	//    colors:    | Fcol, FCol, FCol | Fcol, FCol, FCol | ... | White everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | 0 everywhere, i.e., no smoothing.
	//
	// The positions and indices can simply be copied over. The normals must be triplicated over
	// all three vertices of each triangle. The other tangents are left unset and
	// bRecomputeTangents enabled in the StaticMesh SourceModel settings, see
	// AddRawMeshToStaticMesh. Texture coordinates are set to (0, 0) since the Trimesh doesn't have
	// any texture coordinates.

	FRawMesh RawMesh;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = Trimesh.GetVertexPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = Trimesh.GetVertexPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = Trimesh.GetVertexIndices();

	const int32 NumTriangles = Trimesh.GetNumTriangles();
	const int32 NumIndices = Trimesh.GetNumIndices();

	// Buffers with three elements per triangle, i.e., one per wedge.
	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	// Buffers with one element per triangle.
	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);

	const TArray<FVector> TriangleNormals = Trimesh.GetTriangleNormals();

	for (int32 TIdx = 0; TIdx < NumTriangles; ++TIdx)
	{
		// I don't know how to compute the first two tangents, but bRecomputeTangents (not
		// bRecomputeNormals) has been enabled on the StaticMesh SourceModel in
		// AddRawMeshToStaticMesh. Perhaps that's enough.

		// Since we replicate the same normal for all vertices of a triangle we will get a
		// flat-shaded mesh, should this mesh ever be used for rendering.
		const FVector3f Normal = ToMeshVector(TriangleNormals[TIdx]);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);
		RawMesh.WedgeTangentZ.Add(Normal);

		// The collision mesh doesn't have color information, so just write white.
		const FColor Color(255, 255, 255);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);
		RawMesh.WedgeColors.Add(Color);

		// We must write something to the texture coordinates or else Unreal Engine crashes when
		// processing this mesh later. We could try to do something clever here, but I think just
		// writing zero everywhere is safest.
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});

		// The collision mesh doesn't have material slots, so the best we can do is to provide a
		// single material and apply it to every triangle.
		RawMesh.FaceMaterialIndices.Add(0);

		// Not entirely sure on the FaceSmoothingMasks, the documentation is a little vague:
		//     Smoothing mask. Array[FaceId] = uint32
		// But I believe the process is that Unreal Engine does bitwise-and between two neighboring
		// faces and if the result comes out as non-zero then smoothing will happen along that
		// edge. Not sure what is being smoothed though. Perhaps the vertex normals are merged
		// if smoothing is on, and kept separate if smoothing is off. Also not sure how this
		// relates to the bRecompute.* settings on the StaticMesh's SourceModel.
		RawMesh.FaceSmoothingMasks.Add(0x00000000);
	}

	return RawMesh;
}

FRawMesh FAGX_EditorUtilities::CreateRawMeshFromRenderData(const FRenderDataBarrier& RenderData)
{
	if (RenderData.GetNumTriangles() <= 0)
	{
		// No render mesh data available, this render data is invalid.
		UE_LOG(
			LogAGX, Error,
			TEXT("Did not find any triangle data in imported render data '%s'. Cannot create "
				 "StaticMesh asset."),
			*RenderData.GetGuid().ToString());
		return FRawMesh();
	}

	// What we have:
	//
	// The render data consists of four arrays: positions, normals, texture coordinates, and
	// indices. The function is similar to the collision data, but this time everything is
	// indexed and all arrays except for the index array create a single conceptual Vertex
	// struct.
	//
	// Data shared among triangles:
	//    positions:  [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    normals:    [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//    tex coords: [Vec2, Vec2, Vec2, Vec2, Vec2, ... ]
	//
	// Data owned by each triangle:
	//                |  Triangle 0   |  Triangle 1   | ... |
	//    indices:    | int, int, int | int, int, int | ... |
	//
	//
	// What we want:
	//
	// Unreal Engine store its meshes in a format similar to the render format in AGX Dynamics, but
	// more data is owned per triangle instead of shared between multiple triangles. The Unreal
	// Engine format consists of six arrays: positions, indices, tangent X, tangent Y, tangent Z,
	// and texture coordinates. Tangent Z has the same meaning as the normal in the AGX Dynamics
	// data.
	//
	// Data shared among triangles:
	//   positions: [Vec3, Vec3, Vec3, Vec3, Vec3, ... ]
	//
	// Data owned by each triangle:
	//               |    Triangle 0    |    Triangle 1    | ... |
	//    indices:   | int,  int,  int  | int,  int,  int  | ... | Index into positions.
	//    tangent x: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent y: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Not written, generated.
	//    tangent z: | Vec3, Vec3, Vec3 | Vec3, Vec3, Vec3 | ... | Copied from Render Data.
	//    tex coord: | Vec2, Vec2, Vec2 | Vec2, Vec2, Vec2 | ... | Copied from Render Data.
	//    colors:    | FCol, FCol, FCol | FCol, FCol, FCol | ... | While everywhere.
	//    material:  |       int        |       int        | ... | 0 everywhere.
	//    smoothing: |      uint        |      uint        | ... | All-1 everywhere.

	FRawMesh RawMesh;

	// A straight up copy of the vertex positions may be wasteful since the render data may
	// contain duplicated positions with different normals or texture coordinates. Sine Unreal
	// Engine decouples the normals and the texture coordinates from the positions we may end up
	// with useless position duplicates. If this becomes a serious concern, then find a way to
	// remove duplicates and patch the WedgeIndies to point to the correct merged vertex position.
	// Must use the render vertex indices in the per-index conversion loop below.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	RawMesh.VertexPositions = RenderData.GetPositions();
#else
	const TArray<FVector>& TrimeshVertexPositions = RenderData.GetPositions();
	RawMesh.VertexPositions.SetNum(TrimeshVertexPositions.Num());
	for (int32 I = 0; I < TrimeshVertexPositions.Num(); ++I)
	{
		// May do a double -> float conversion, depending on the UE_LARGE_WORLD_COORDINATES_DISABLED
		// preprocessor macro.
		RawMesh.VertexPositions[I] = ToMeshVector(TrimeshVertexPositions[I]);
	}
#endif
	RawMesh.WedgeIndices = RenderData.GetIndices();

	const int32 NumTriangles = RenderData.GetNumTriangles();
	const int32 NumIndices = RenderData.GetNumIndices();

	RawMesh.WedgeTangentZ.Reserve(NumIndices);
	RawMesh.WedgeColors.Reserve(NumIndices);
	RawMesh.WedgeTexCoords[0].Reserve(NumIndices);

	const TArray<FVector> RenderNormals = RenderData.GetNormals();
	const auto RenderTexCoords = RenderData.GetTextureCoordinates();

	for (int32 I = 0; I < NumIndices; ++I)
	{
		const int32 RenderI = RawMesh.WedgeIndices[I];
		RawMesh.WedgeTangentZ.Add(ToMeshVector(RenderNormals[RenderI]));
		// Not all Render Data has texture coordinates.
		if (RenderTexCoords.Num() > I)
		{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			RawMesh.WedgeTexCoords[0].Add(RenderTexCoords[RenderI]);
#else
			RawMesh.WedgeTexCoords[0].Add(
				FVector2f {(float) RenderTexCoords[RenderI].X, (float) RenderTexCoords[RenderI].Y});
#endif
		}
		else
		{
			RawMesh.WedgeTexCoords[0].Add({0.0f, 0.0f});
		}
		RawMesh.WedgeColors.Add(FColor(255, 255, 255));
	}

	RawMesh.FaceMaterialIndices.Reserve(NumTriangles);
	RawMesh.FaceSmoothingMasks.Reserve(NumTriangles);
	for (int32 I = 0; I < NumTriangles; ++I)
	{
		RawMesh.FaceMaterialIndices.Add(0);
		RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
	}

	return RawMesh;
}

void FAGX_EditorUtilities::AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh)
{
#if UE_VERSION_OLDER_THAN(4, 27, 0)
	StaticMesh->StaticMaterials.Add(FStaticMaterial());
#else
	StaticMesh->GetStaticMaterials().Add(FStaticMaterial());
#endif

#if UE_VERSION_OLDER_THAN(4, 23, 0)
	StaticMesh->SourceModels.Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
#elif UE_VERSION_OLDER_THAN(5, 0, 0)
	StaticMesh->GetSourceModels().Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModels().Last();
#else
	FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
#endif

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	// There is a SaveRawMesh on the source model as well, but calling that causes a failed assert.
	// Is that a sign that we're doing something we shouldn't and the engine doesn't detect it
	// because we're sidestepping the safety checks? Or is it OK to do it this way?
	SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
#else
	SourceModel.SaveRawMesh(RawMesh);
#endif

	FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;

	// Somewhat unclear what all these should be. Setting everything I don't understand to false.
	BuildSettings.bRecomputeNormals = false;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = true;
	BuildSettings.bGenerateLightmapUVs = true;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	BuildSettings.bBuildAdjacencyBuffer = false;
#endif
	BuildSettings.bBuildReversedIndexBuffer = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;
}

AAGX_ConstraintActor* FAGX_EditorUtilities::CreateConstraintActor(
	UClass* ConstraintType, UAGX_RigidBodyComponent* RigidBody1,
	UAGX_RigidBodyComponent* RigidBody2, bool bInPlayingWorldIfAvailable, bool bSelect,
	bool bShowNotification)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();

	check(World);
	check(ConstraintType->IsChildOf<AAGX_ConstraintActor>());

	// Create the new Constraint Actor.
	AAGX_ConstraintActor* NewActor =
		World->SpawnActorDeferred<AAGX_ConstraintActor>(ConstraintType, FTransform::Identity);

	check(NewActor);

	/// \todo We have the Component we want. There should be a way to specify it directly, without
	/// being dependent on its name.
	UAGX_ConstraintComponent* Constraint = NewActor->GetConstraintComponent();
	Constraint->BodyAttachment1.RigidBody.OwningActor = RigidBody1->GetOwner();
	Constraint->BodyAttachment1.RigidBody.BodyName = RigidBody1->GetFName();
	if (RigidBody2 != nullptr)
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = RigidBody2->GetOwner();
		Constraint->BodyAttachment2.RigidBody.BodyName = RigidBody2->GetFName();
	}
	else
	{
		Constraint->BodyAttachment2.RigidBody.OwningActor = nullptr;
		Constraint->BodyAttachment2.RigidBody.BodyName = NAME_None;
	}

	NewActor->FinishSpawning(FTransform::Identity, true);

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		ShowNotification(LOCTEXT("CreateConstraintSucceeded", "AGX Constraint Created"));
	}

	return NewActor;
}

AAGX_ConstraintFrameActor* FAGX_EditorUtilities::CreateConstraintFrameActor(
	AActor* ParentActor, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();
	check(World);

	// Create the new Constraint Frame Actor.
	AAGX_ConstraintFrameActor* NewActor = World->SpawnActor<AAGX_ConstraintFrameActor>();
	check(NewActor);

	// Set the new actor as child to the Rigid Body.
	if (ParentActor)
	{
		if (ParentActor->GetWorld() == World)
		{
			NewActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("Failed to attach the new AGX Constraint Frame Actor to the specified "
					 "Parent Rigid Body Actor, because it is in another World."));
		}
	}

	if (bSelect)
	{
		SelectActor(NewActor);
	}

	if (bShowNotification)
	{
		ShowNotification(
			LOCTEXT("CreateConstraintFrameActorSucceded", "AGX Constraint Frame Actor Created"));
	}

	return NewActor;
}

void FAGX_EditorUtilities::SelectActor(AActor* Actor, bool bDeselectPrevious)
{
	if (bDeselectPrevious)
	{
		GEditor->SelectNone(
			/*bNoteSelectionChange*/ false,
			/*bDeselectBSPSurfs*/ true,
			/*WarnAboutManyActors*/ false);
	}

	if (Actor)
	{
		GEditor->SelectActor(
			Actor,
			/*bInSelected*/ true,
			/*bNotify*/ false);
	}

	GEditor->NoteSelectionChange();
}

void FAGX_EditorUtilities::ShowNotification(const FText& Text)
{
	FNotificationInfo Info(Text);
	Info.Image = FAGX_EditorUtilities::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 5.0f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();
	// GEditor->PlayEditorSound(CompileSuccessSound);
}

UWorld* FAGX_EditorUtilities::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

UWorld* FAGX_EditorUtilities::GetPlayingWorld()
{
	// Without starting from an Actor, the world needs to be found
	// in another way:

	TArray<APlayerController*> PlayerControllers;
	GEngine->GetAllLocalPlayerControllers(PlayerControllers);

	if (PlayerControllers.Num() > 0)
	{
		return PlayerControllers[0]->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

UWorld* FAGX_EditorUtilities::GetCurrentWorld()
{
	if (UWorld* PlayingWorld = GetPlayingWorld())
	{
		return PlayingWorld;
	}
	else
	{
		return GetEditorWorld();
	}
}

void FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
	AActor** OutActor1, AActor** OutActor2, bool bSearchSubtrees, bool bSearchAncestors)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();

	if (!SelectedActors)
		return;

	if (OutActor1)
		*OutActor1 = nullptr;

	if (OutActor2)
		*OutActor2 = nullptr;

	// Assigns to first available of OutActor1 and OutActor2, and returns whether
	// at least one of them is afterwards still available for assignment.
	auto AssignOutActors = [OutActor1, OutActor2](AActor* RigidBodyActor)
	{
		if (OutActor1 && *OutActor1 == nullptr)
		{
			*OutActor1 = RigidBodyActor;
		}
		// Making sure same actor is not used for both OutActors.
		else if (OutActor2 && *OutActor2 == nullptr && (!OutActor1 || *OutActor1 != RigidBodyActor))
		{
			*OutActor2 = RigidBodyActor;
		}

		return (OutActor1 && *OutActor1 == nullptr) || (OutActor2 && *OutActor2 == nullptr);
	};

	// Search the selected actors fpr matching actors. Doing this step completely before
	// start searching in subtrees, in case selected actors are in each others subtrees.
	for (int32 i = 0; i < SelectedActors->Num(); ++i)
	{
		if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
		{
			if (UAGX_RigidBodyComponent::GetFirstFromActor(SelectedActor))
			{
				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(SelectedActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's subtree for matching actors. Only one matching actor
	// allowed per selected actor subtree.
	if (bSearchSubtrees)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor =
					GetRigidBodyActorFromSubtree(SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}

	// Search each selected actor's ancestor chain for matching actors. Only one matching actor
	// allowed per selected actor ancestor chain.
	if (bSearchAncestors)
	{
		for (int32 i = 0; i < SelectedActors->Num(); ++i)
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetSelectedObject(i)))
			{
				AActor* RigidBodyActor = GetRigidBodyActorFromAncestors(
					SelectedActor, (OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromSubtree(
	AActor* SubtreeRoot, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (SubtreeRoot)
	{
		if (SubtreeRoot != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(SubtreeRoot))
		{
			RigidBodyActor = SubtreeRoot; // found it
		}
		else
		{
			TArray<AActor*> AttachedActors;
			SubtreeRoot->GetAttachedActors(AttachedActors);

			for (AActor* AttachedActor : AttachedActors)
			{
				RigidBodyActor = GetRigidBodyActorFromSubtree(AttachedActor, IgnoreActor);

				if (RigidBodyActor)
				{
					break; // found it
				}
			}
		}
	}

	return RigidBodyActor;
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromAncestors(
	AActor* Actor, const AActor* IgnoreActor)
{
	AActor* RigidBodyActor = nullptr;

	if (Actor)
	{
		if (Actor != IgnoreActor && UAGX_RigidBodyComponent::GetFirstFromActor(Actor))
		{
			RigidBodyActor = Actor;
		}
		else
		{
			RigidBodyActor =
				GetRigidBodyActorFromAncestors(Actor->GetAttachParentActor(), IgnoreActor);
		}
	}

	return RigidBodyActor;
}

void FAGX_EditorUtilities::GetAllClassesOfType(
	TArray<UClass*>& OutMatches, UClass* BaseClass, bool bIncludeAbstract)
{
	for (TObjectIterator<UClass> ClassItr; ClassItr; ++ClassItr)
	{
		UClass* Class = *ClassItr;

		if (Class && Class->IsChildOf(BaseClass))
		{
			if (bIncludeAbstract || !Class->HasAnyClassFlags(CLASS_Abstract))
			{
				OutMatches.Add(Class);
			}
		}
	}
}

EVisibility FAGX_EditorUtilities::VisibleIf(bool bVisible)
{
	return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FString FAGX_EditorUtilities::SelectExistingFileDialog(
	const FString& FileDescription, const FString& FileExtension)
{
	const FString DialogTitle = FString("Select a ") + FileDescription;
	const FString FileTypes = FileDescription + FString("|*") + FileExtension;
	// For a discussion on window handles see
	// https://answers.unrealengine.com/questions/395516/opening-a-file-dialog-from-a-plugin.html
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->OpenFileDialog(
		nullptr, DialogTitle, TEXT("DefaultPath"), TEXT("DefaultFile"), FileTypes,
		EFileDialogFlags::None, Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Log, TEXT("No %s file selected. Doing nothing."), *FileExtension);
		return "";
	}
	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("Multiple files selected but only single file selection supported. Doing "
				 "nothing."));
		FAGX_EditorUtilities::ShowNotification(LOCTEXT(
			"Multiple files",
			"Multiple files selected but but only single file selection supported. Doing "
			"nothing."));
		return "";
	}
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Filenames[0]);
}

FString FAGX_EditorUtilities::SelectExistingDirectoryDialog(
	const FString& DialogTitle, const FString& InStartDir, bool AllowNoneSelected)
{
	const FString StartDir = InStartDir.IsEmpty() ? FString("DefaultPath") : InStartDir;
	FString DirectoryPath("");
	bool DirectorySelected = FDesktopPlatformModule::Get()->OpenDirectoryDialog(
		nullptr, DialogTitle, StartDir, DirectoryPath);

	if (!AllowNoneSelected && (!DirectorySelected || DirectoryPath.IsEmpty()))
	{
		UE_LOG(LogAGX, Log, TEXT("No directory selected. Doing nothing."));
		return "";
	}

	return DirectoryPath;
}

FString FAGX_EditorUtilities::SelectNewFileDialog(
	const FString& DialogTitle, const FString& FileExtension, const FString& FileTypes,
	const FString& DefaultFile, const FString& InStartDir)
{
	TArray<FString> Filenames;
	bool FileSelected = FDesktopPlatformModule::Get()->SaveFileDialog(
		nullptr, DialogTitle, InStartDir, DefaultFile, FileTypes, EFileDialogFlags::None,
		Filenames);
	if (!FileSelected || Filenames.Num() == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("No file selected, doing nothing."));
		return "";
	}

	if (Filenames.Num() > 1)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Multiple files selected but we only support selecting one. Doing nothing."));
		ShowNotification(LOCTEXT(
			"Multiple files",
			"Multiple files selected but we only support single files for now. Doing "
			"nothing."));
		return "";
	}

	const FString Filename = Filenames[0];
	if (Filename.IsEmpty())
	{
		UE_LOG(LogAGX, Warning, TEXT("Selected file has empty file name. Doing nothing."));
		return "";
	}

	return FPaths::ConvertRelativePathToFull(Filename);
}

void FAGX_EditorUtilities::SaveAndCompile(UBlueprint& Blueprint)
{
	FKismetEditorUtilities::CompileBlueprint(&Blueprint);
	FAGX_ObjectUtilities::SaveAsset(Blueprint);
}

#undef LOCTEXT_NAMESPACE
