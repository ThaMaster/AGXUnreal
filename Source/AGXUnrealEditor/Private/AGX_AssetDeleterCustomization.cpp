

/// @todo Experimental code, do not merge to master.
#pragma message("Experimental code, do not merge to master.")


#include "AGX_AssetDeleterCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_AssetDeleter.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetRegistryModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Engine.h"
#include "Misc/EngineVersionComparison.h"
#include "ObjectTools.h"
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "Serialization/FindReferencersArchive.h"
#include "Utilities/AGX_EditorUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_AssetDeleterCustomization"

TSharedRef<IDetailCustomization> FAGX_AssetDeleterCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AssetDeleterCustomization);
}

#if 0
namespace AGX_AssetDeleterCustomization_helpers
{
	/**
	 * Delete function that marks the deleted object pending kill. The actual delete will happen
	 * later, during garbage collection. May trigger an immediate garbage collection with a call to
	 * GEngine->ForceGarbageCollection(true|false). Not sure what the argument should be.
	 *
	 * This version is based on text in the official documentation:
	 *
	 * > In addition, the function `MarkPendingKill()` can be called directly on an Object. This
	 * > function sets all pointers to the Object to `NULL` and removes the Object from global
	 * > searches. The Object is fully deleted on the next garbage collection pass.
	 *
	 * https://docs.unrealengine.com/5.1/en-US/objects-in-unreal-engine/
	 *
	 * The above appears to be a lie. Or at least incomplete.
	 *
	 * It causes a crash soon after:
	 *
	 * Signal 11 caught.
	 * [LogCore: === Critical error: ===
	 * Unhandled Exception: SIGSEGV: unaligned memory access (SIMD vectors?)
	 *
	 * LogCore: Fatal error!
	 */
	// clang-format off
	/**
	 * 0x00007fa8cdaa1fa6 libUE4Editor-VulkanRHI.so!FVulkanCommandListContext::SetShaderUniformBuffer(ShaderStage::EStage, FVulkanUniformBuffer const*, int, FVulkanShader const*) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/VulkanRHI/Private/VulkanCommands.cpp:531]
	 * 0x00007fa8cda8cb9d libUE4Editor-VulkanRHI.so!FVulkanCommandListContext::RHISetShaderUniformBuffer(FRHIGraphicsShader*, unsigned int, FRHIUniformBuffer*) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/VulkanRHI/Private/VulkanCommands.cpp:567]
	 * 0x00007fa9ae5e8d00 libUE4Editor-Engine.so!FRHICommand<FRHICommandSetShaderUniformBuffer<FRHIGraphicsShader>, FRHICommandSetShaderUniformBufferString>::ExecuteAndDestruct(FRHICommandListBase&, FRHICommandListDebugContext&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Public/RHICommandList.h:765]
	 * 0x00007fa9ab3ad590 libUE4Editor-RHI.so!FRHICommandListExecutor::ExecuteInner_DoExecute(FRHICommandListBase&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:372]
	 * 0x00007fa9ab3aee39 libUE4Editor-RHI.so!FRHICommandListExecutor::ExecuteInner(FRHICommandListBase&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:657]
	 * 0x00007fa9ab3b15af libUE4Editor-RHI.so!FRHICommandListExecutor::ExecuteList(FRHICommandListBase&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:681]
	 * 0x00007fa9ab3dc6dd libUE4Editor-RHI.so!FRHICommandListBase::~FRHICommandListBase() [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:922]
	 * 0x00007fa9ab41dde6 libUE4Editor-RHI.so!FRHICommandWaitForAndSubmitSubList::Execute(FRHICommandListBase&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:1108]
	 * 0x00007fa9ab41d3e7 libUE4Editor-RHI.so!FRHICommand<FRHICommandWaitForAndSubmitSubList, FRHICommandWaitForAndSubmitSubListString1074>::ExecuteAndDestruct(FRHICommandListBase&, FRHICommandListDebugContext&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Public/RHICommandList.h:765]
	 * 0x00007fa9ab3ad590 libUE4Editor-RHI.so!FRHICommandListExecutor::ExecuteInner_DoExecute(FRHICommandListBase&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:372]
	 * 0x00007fa9ab43327c libUE4Editor-RHI.so!FExecuteRHIThreadTask::DoTask(ENamedThreads::Type, TRefCountPtr<FGraphEvent> const&) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RHI/Private/RHICommandList.cpp:427]
	 * 0x00007fa9ab432803 libUE4Editor-RHI.so!TGraphTask<FExecuteRHIThreadTask>::ExecuteTask(TArray<FBaseGraphTask*, TSizedDefaultAllocator<32> >&, ENamedThreads::Type) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/Core/Public/Async/TaskGraphInterfaces.h:886]
	 * 0x00007fa9b0de129b libUE4Editor-Core.so!FNamedTaskThread::ProcessTasksNamedThread(int, bool) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/Core/Private/Async/TaskGraph.cpp:710]
	 * 0x00007fa9b0ddf77e libUE4Editor-Core.so!FNamedTaskThread::ProcessTasksUntilQuit(int) [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/Core/Private/Async/TaskGraph.cpp:601]
	 * 0x00007fa9ab5f9bc3 libUE4Editor-RenderCore.so!FRHIThread::Run() [/media/i2000/UnrealEngine_4.27_Build/Engine/Source/Runtime/RenderCore/Private/RenderingThread.cpp:319]
	 */
	// clang-format on
	void DeleteAsset_MarkPendingKill(UObject* ToDelete)
	{
		// Delete code goes here.
		ToDelete->MarkPendingKill();
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

	void DeleteAsset_LikeBlueprintEditorTestsCleanup(UObject* ToDelete)
	{
		const FString AssetPath = ToDelete->GetPathName();
		UE_LOG(LogAGX, Warning, TEXT("FullPath='%s'"), *AssetPath);
		const FString BasePath = [&AssetPath]()
		{
			FString Result;
			AssetPath.Split(
				TEXT("."), &Result, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			return Result + TEXT(".uasset");
		}();
		const FString RelativeFileSystemPath = FPackageName::LongPackageNameToFilename(BasePath);
		const FString FileSystemPath = FPaths::ConvertRelativePathToFull(RelativeFileSystemPath);

		UE_LOG(LogAGX, Warning, TEXT("AssetPath='%s'"), *AssetPath);
		UE_LOG(LogAGX, Warning, TEXT("BasePath='%s'"), *BasePath);
		UE_LOG(LogAGX, Warning, TEXT("FilesystemPath='%s'"), *FileSystemPath);

		if (!IFileManager::Get().FileExists(*FileSystemPath))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Want to delete asset '%s' but the corresponding file '%s' doesn't exist. "
					 "Doing nothing."),
				*AssetPath, *FileSystemPath);
			return;
		}

#if 1
		IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();

		if (GEditor != nullptr)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(ToDelete);
		}

		AssetRegistry.AssetDeleted(ToDelete);
		NullReferencesToObject(ToDelete);
		ObjectTools::DeleteSingleObject(ToDelete);
		IFileManager::Get().Delete(*FileSystemPath);
#endif
	}

	/**
	 * Dispatch function to make it easy to switch between different delete implementations.
	 * Only one call should be uncommented.
	 */
	void DeleteAsset(UObject* ToDelete)
	{
		// DeleteAsset_MarkPendingKill(ToDelete);
		DeleteAsset_LikeBlueprintEditorTestsCleanup(ToDelete);
	}
}
#endif

void FAGX_AssetDeleterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Asset Deleter");

	// clang-format off
	CategoryBuilder.AddCustomRow(LOCTEXT("AssetDeleter", "Asset Deleter"))
	[
		SNew(SButton)
		.Text(LOCTEXT("DeleteAsset", "Delete Selected Asset"))
		.OnClicked_Lambda([&DetailBuilder]()
		{
			UE_LOG(LogAGX, Warning, TEXT("Delete Asset clicked"));
			bool AnyDeleted = false;
			TArray<TWeakObjectPtr<UObject>> SelectedObjects;
			DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
			for (TWeakObjectPtr<UObject> ObjectPtr : SelectedObjects)
			{
				if (!ObjectPtr.IsValid())
				{
					UE_LOG(LogAGX, Warning, TEXT("Found invalid/nullptr selected object. Ignoring."));
					continue;
				}
				UAGX_AssetDeleter* Deleter = Cast<UAGX_AssetDeleter>(ObjectPtr.Get());
				if (Deleter == nullptr)
				{
					UE_LOG(LogAGX, Warning, TEXT("Found non-deleter selected object. Ignoring."));
					continue;
				}
				UObject* ToDelete = Deleter->ToDelete;
				if (ToDelete == nullptr)
				{
					UE_LOG(LogAGX, Warning, TEXT("Found deleter with nullptr asset. Ignoring."));
					continue;
				}
				UE_LOG(LogAGX, Warning, TEXT("Deleting '%s'."), *ToDelete->GetPathName());

#if 0
				AGX_AssetDeleterCustomization_helpers::DeleteAsset(ToDelete);
#else
				FAGX_EditorUtilities::DeleteAsset(*ToDelete);
#endif

				AnyDeleted = true;
			}

			/// \todo Test without this part.
			if (AnyDeleted)
			{
				UE_LOG(LogAGX, Warning, TEXT("At least one asset deleted, running garbage collection."));
				if (GEngine != nullptr)
				{
					GEngine->ForceGarbageCollection(true);
				}
				else
				{
                    UE_LOG(LogAGX, Warning, TEXT("GEngine is nullptr, cannot run garbage collection."));
				}
			}


			return FReply::Handled();
		})
	];
	// clang-format on
}
