// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_ImporterToEditor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImporterSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Import/AGX_SCSNodeCollection.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/SCS_Node.h"
#include "FileHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PackageTools.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/MetaData.h"

// Standard library includes.
#include <type_traits>

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

	void WriteImportTag(UActorComponent& Component, const FGuid& SessionGuid)
	{
		Component.ComponentTags.Empty();
		Component.ComponentTags.Add(*SessionGuid.ToString());
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

	template <typename T>
	FString GetAssetTypeFromType(const UObject& Asset)
	{
		if constexpr (std::is_same_v<T, UAGX_MergeSplitThresholdsBase>)
			return FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName();

		if constexpr (std::is_same_v<T, UStaticMesh>)
		{
			if (Asset.GetName().Contains("Collision"))
				return FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName();
			else if (Asset.GetName().Contains("Render"))
				return FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName();
			else
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("GetAssetTypeFromType called with StaticMesh with unsupported name '%s'. "
						 "This may cause errors during import/reimport."),
					*Asset.GetName());
				return "Unsupported";
			}
		}

		if constexpr (std::is_same_v<T, UMaterialInterface>)
			return FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName();
	}

	FString GetModelDirectoryPathFromBaseBlueprint(UBlueprint& BaseBP)
	{
		const FString ParentDir = FPaths::GetPath(BaseBP.GetPathName());
		if (!FPaths::GetBaseFilename(ParentDir).Equals("Blueprint"))
		{
			return "";
		}

		return FPaths::GetPath(ParentDir);
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

	void CopyPropertyRecursive(
		const void* Archetype, const void* Source, void* OutDest, const FProperty* Property,
		const TMap<UObject*, UObject*>& TransientToAsset)
	{
		if (Property == nullptr || !Property->HasAnyPropertyFlags(CPF_Edit))
			return;

		const void* ArchetypeVal = Property->ContainerPtrToValuePtr<void>(Archetype);
		const void* SourceVal = Property->ContainerPtrToValuePtr<void>(Source);
		void* DestVal = Property->ContainerPtrToValuePtr<void>(OutDest);

		if (Property->Identical(SourceVal, DestVal))
			return; // Nothing to do, already equal.

		if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
		{
			UObject* SourceObject = ObjectProperty->GetObjectPropertyValue(SourceVal);
			if (UObject* Asset = TransientToAsset.FindRef(SourceObject))
			{
				const bool ShouldCopy =
					Archetype == OutDest || Property->Identical(ArchetypeVal, DestVal);

				if (ShouldCopy)
					ObjectProperty->SetObjectPropertyValue(DestVal, Asset);

				return;
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			const void* StructArchetype = StructProperty->ContainerPtrToValuePtr<void>(Archetype);
			const void* StructSource = StructProperty->ContainerPtrToValuePtr<void>(Source);
			void* StructDest = StructProperty->ContainerPtrToValuePtr<void>(OutDest);

			// Recursively copy properties within the struct.
			for (TFieldIterator<FProperty> StructPropIt(StructProperty->Struct); StructPropIt;
				 ++StructPropIt)
			{
				FProperty* StructPropertyField = *StructPropIt;
				CopyPropertyRecursive(
					StructArchetype, StructSource, StructDest, StructPropertyField,
					TransientToAsset);
			}
			return;
		}

		const bool ShouldCopy = Archetype == OutDest || Property->Identical(ArchetypeVal, DestVal);
		if (ShouldCopy)
			Property->CopyCompleteValue(DestVal, SourceVal);
	}

	// Similar to FAGX_ObjectUtilities::CopyProperties, but with some special handling for the
	// TransientToAsset map.
	bool CopyProperties(
		const UObject& Source, UObject& OutDest, const TMap<UObject*, UObject*>& TransientToAsset)
	{
		UClass* Class = Source.GetClass();
		if (OutDest.GetClass() != Class)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Tried to copy properties from object '%s' of type '%s' to object '%s' of "
					 "type '%s'. Types must match."),
				*Source.GetName(), *Source.GetClass()->GetName(), *OutDest.GetName(),
				*OutDest.GetClass()->GetName());
			return false;
		}

		TArray<UObject*> ArchetypeInstances;
		OutDest.GetArchetypeInstances(ArchetypeInstances);
		for (UObject* Instance : ArchetypeInstances)
		{
			for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
			{
				const FProperty* Property = *PropIt;
				CopyPropertyRecursive(&OutDest, &Source, Instance, Property, TransientToAsset);
			}
		}

		for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
		{
			const FProperty* Property = *PropIt;
			CopyPropertyRecursive(&OutDest, &Source, &OutDest, Property, TransientToAsset);
		}

		return true;
	}

	bool ValidateImportEnum(EAGX_ImportResult Result)
	{
		if (IsUnrecoverableError(Result))
		{
			const FString Text =
				FString::Printf(TEXT("Errors occurred during import and the result may not be the "
									 "expected result. Log category LogAGX in the Output Log may "
									 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		if (Result != EAGX_ImportResult::Success)
		{
			const FString Text = FString::Printf(
				TEXT("Some issues occurred during import and the result may not be the "
					 "expected result. Log category LogAGX in the Output Log may "
					 "contain more information."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		return true;
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

		return ValidateImportEnum(Result.Result);
	}

	bool ValidateBlueprintForReimport(const UBlueprint& Bp)
	{
		if (Bp.SimpleConstructionScript->FindSCSNode(TEXT("AGX_ModelSource")) == nullptr)
		{
			const FString Text = FString::Printf(TEXT(
				"Unable to find a valid AGX Mode Source Component in the selected Blueprint."));
			FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
			return false;
		}

		return true;
	}

	void RemoveDeletedComponents(UBlueprint& Blueprint, const FGuid& SessionGuid)
	{
		auto HasMatchingTag = [&SessionGuid](const UActorComponent& Component)
		{
			if (Component.ComponentTags.Num() == 0)
				return false;

			return FGuid(Component.ComponentTags[0].ToString()) == SessionGuid;
		};

		// Any Component that does not have the current SessionGuid tag, we know is not part of the
		// newly imported model and should be removed.
		const TArray<USCS_Node*> Nodes = Blueprint.SimpleConstructionScript->GetAllNodes();
		for (USCS_Node* Node : Nodes)
		{
			if (Node == nullptr || Node->ComponentTemplate == nullptr)
			{
				AGX_CHECK(false);
				continue;
			}

			if (!HasMatchingTag(*Node->ComponentTemplate))
				Blueprint.SimpleConstructionScript->RemoveNodeAndPromoteChildren(Node);
		}
	}

	void RemoveDeletedAssets(const FString& RootDirectory, const FGuid& SessionGuid)
	{
		auto HasMatchingSessionGuid = [&SessionGuid](const UObject& Object)
		{
			UMetaData* MetaData = Object.GetOutermost()->GetMetaData();
			const FString GuidStr = MetaData->GetValue(&Object, TEXT("AGX_ImportSessionGuid"));
			return FGuid(GuidStr) == SessionGuid;
		};

		TArray<UObject*> AssetsToDelete;

		auto CollectForRemoval = [&](auto Assets)
		{
			for (auto Asset : Assets)
			{
				if (!HasMatchingSessionGuid(*Asset))
					AssetsToDelete.Add(Asset);
			}
		};

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UMaterialInterface>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UStaticMesh>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName())));

		CollectForRemoval(FAGX_EditorUtilities::FindAssets<UStaticMesh>(FPaths::Combine(
			RootDirectory, FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName())));

		CollectForRemoval(
			FAGX_EditorUtilities::FindAssets<UAGX_MergeSplitThresholdsBase>(FPaths::Combine(
				RootDirectory,
				FAGX_ImportUtilities::GetImportMergeSplitThresholdsDirectoryName())));

		FAGX_EditorUtilities::DeleteImportedAssets(AssetsToDelete);
	}

	void AddSessionGuid(const FGuid& SessionGuid, UObject& OutObject)
	{
		UMetaData* MetaData = OutObject.GetOutermost()->GetMetaData();
		MetaData->SetValue(&OutObject, TEXT("AGX_ImportSessionGuid"), *SessionGuid.ToString());
	}

	void WriteAssetToDisk(
		const FString& RootDir, const FString& AssetType, UObject& Object, const FGuid& SessionGuid)
	{
		const FString AssetName = Object.GetName();
		const FString PackagePath =
			FPaths::Combine(FAGX_ImportUtilities::CreatePackagePath(RootDir, AssetType), AssetName);
		UPackage* Package = CreatePackage(*PackagePath);
		Object.Rename(*AssetName, Package);
		Package->MarkPackageDirty();
		Object.SetFlags(RF_Public | RF_Standalone);

		AddSessionGuid(SessionGuid, Object);
		FAGX_ObjectUtilities::SaveAsset(Object);
	}

	void WriteAssetsToDisk(const FString& RootDir, const FAGX_ImportContext* Context)
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
					WriteAssetToDisk(RootDir, AssetType, *SCMST, Context->SessionGuid);
			}
		}

		if (Context->RenderMaterials != nullptr)
		{
			const FString AssetType = FAGX_ImportUtilities::GetImportRenderMaterialDirectoryName();
			for (const auto& [Guid, Rm] : *Context->RenderMaterials)
			{
				WriteAssetToDisk(RootDir, AssetType, *Rm, Context->SessionGuid);
			}
		}

		if (Context->RenderStaticMeshes != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportRenderStaticMeshDirectoryName();
			for (const auto& [Guid, Sm] : *Context->RenderStaticMeshes)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sm, Context->SessionGuid);
			}
		}

		if (Context->CollisionStaticMeshes != nullptr)
		{
			const FString AssetType =
				FAGX_ImportUtilities::GetImportCollisionStaticMeshDirectoryName();
			for (const auto& [Guid, Sm] : *Context->CollisionStaticMeshes)
			{
				WriteAssetToDisk(RootDir, AssetType, *Sm, Context->SessionGuid);
			}
		}
	}

	template <typename T, typename = void>
	struct HasImportGuid : std::false_type
	{
	};

	template <typename T>
	struct HasImportGuid<T, std::void_t<decltype(std::declval<T>().ImportGuid)>> : std::true_type
	{
	};

	/**
	 * Given a Component that has been reimported, but is not part of the Blueprint that the
	 * SCSNodeCollection represents, this function tries to find the USCS_Node corresponding to the
	 * reimported Component's attach parent.
	 * Some Components will always be attached to root, and we can use that fact to simplify this
	 * function.
	 */
	template <typename TComponent>
	USCS_Node* GetCorrespondingAttachParent(
		const UBlueprint& Bp, const FAGX_SCSNodeCollection& Nodes,
		const TComponent& ReimportedComponent)
	{
		USceneComponent* Parent = ReimportedComponent.GetAttachParent();
		AGX_CHECK(Parent != nullptr);
		if (Parent == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Found no attach parent to reimported Component '%s', returning root."),
				*ReimportedComponent.GetName());
			return Nodes.RootComponent;
		}

		if constexpr (std::is_same_v<TComponent, UAGX_ShapeComponent>)
		{
			// A Shape can have a RigidBody or Root as parent.
			if (auto Body = Cast<UAGX_RigidBodyComponent>(Parent))
				return Nodes.RigidBodies.FindRef(Body->ImportGuid);
		}

		if constexpr (std::is_same_v<TComponent, UStaticMeshComponent>)
		{
			// A StaticMesh can have a Shape, RigidBody, StaticMesh or Root as parent.
			if (auto Shape = Cast<UAGX_ShapeComponent>(Parent))
				return Nodes.Shapes.FindRef(Shape->ImportGuid);
			if (auto Body = Cast<UAGX_RigidBodyComponent>(Parent))
				return Nodes.RigidBodies.FindRef(Body->ImportGuid);
			if (auto Smc = Cast<UStaticMeshComponent>(Parent))
			{
				if (auto S = Bp.SimpleConstructionScript->FindSCSNode(*Parent->GetName()))
					return S;
			}
		}

		return Nodes.RootComponent; // Default.
	}

	template <typename TComponent>
	USCS_Node* FindNodeAndResolveConflicts(
		const FGuid& Guid, const TComponent& ReimportedComponent,
		const FAGX_SCSNodeCollection& Nodes, TMap<FGuid, USCS_Node*>& OutGuidToNode,
		UBlueprint& OutBlueprint)
	{
		const FName Name(*ReimportedComponent.GetName());
		USCS_Node* Node = OutGuidToNode.FindRef(Guid);

		// Resolve name collisions.
		USCS_Node* NameCollNode = OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
		if (Node != nullptr && NameCollNode != nullptr && NameCollNode != Node)
			NameCollNode->SetVariableName(*FAGX_ImportUtilities::GetUnsetUniqueImportName());

		return Node;
	}

	template <>
	USCS_Node* FindNodeAndResolveConflicts<UStaticMeshComponent>(
		const FGuid& Guid, const UStaticMeshComponent& ReimportedComponent,
		const FAGX_SCSNodeCollection& Nodes, TMap<FGuid, USCS_Node*>& OutGuidToNode,
		UBlueprint& OutBlueprint)
	{
		// StaticMeshComponents we look up by using the name which includes the guid.
		// We expect no conflicts.
		const FName Name(*ReimportedComponent.GetName());
		return OutBlueprint.SimpleConstructionScript->FindSCSNode(Name);
	}

	template <typename TComponent>
	USCS_Node* GetOrCreateNode(
		const FGuid& Guid, const TComponent& ReimportedComponent,
		const FAGX_SCSNodeCollection& Nodes, TMap<FGuid, USCS_Node*>& OutGuidToNode,
		UBlueprint& OutBlueprint)
	{
		const FName Name(*ReimportedComponent.GetName());
		USCS_Node* Node = FindNodeAndResolveConflicts(
			Guid, ReimportedComponent, Nodes, OutGuidToNode, OutBlueprint);

		USCS_Node* Parent = GetCorrespondingAttachParent(OutBlueprint, Nodes, ReimportedComponent);
		AGX_CHECK(Parent != nullptr);
		if (Parent == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find corresponding attach parent for component '%s'."),
				*ReimportedComponent.GetName());
			return nullptr;
		}

		if (Node == nullptr)
		{
			Node = OutBlueprint.SimpleConstructionScript->CreateNode(
				ReimportedComponent.GetClass(), Name);

			Parent->AddChildNode(Node);
		}
		else if (!Node->GetVariableName().IsEqual(Name)) // Node existed.
		{
			// We don't need to handle transform explicitly here, it is copied over later to the
			// component template owned by this node.
			FAGX_BlueprintUtilities::ReParentNode(OutBlueprint, *Node, *Parent, false);
			Node->SetVariableName(Name);
		}

		return Node;
	}

	/**
	 * The FAGX_Importer only creates the Model Source Component but does not populate
	 * its contents since it cannot know the editor only information that is needed by
	 * e.g. render materials.
	 */
	EAGX_ImportResult FinalizeModelSourceComponent(const FAGX_ImportContext& Context, const FString& RootDir)
	{
		UAGX_ModelSourceComponent* Component = Context.ModelSourceComponent;

		if (Component == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("FinalizeModelSourceComponent called, but a ModelSourceComponent could not be "
					 "found in the given FAGX_ImportContext."));
			return EAGX_ImportResult::FatalError;
		}

		Component->FilePath = Context.Settings->FilePath;
		Component->bIgnoreDisabledTrimeshes = Context.Settings->bIgnoreDisabledTrimeshes;

		for (const auto& [Guid, CollisionComponent] : *Context.CollisionStaticMeshCom)
		{
			const FString Name = CollisionComponent->GetName();
			Component->StaticMeshComponentToOwningTrimesh.Add(Name, Guid);
		}

		for (const auto& [Guid, RenderComponent] : *Context.RenderStaticMeshCom)
		{
			const FString Name = RenderComponent->GetName();
			Component->StaticMeshComponentToOwningShape.Add(Name, Guid);
		}

		for (const auto& [Guid, Material] : *Context.RenderMaterials)
		{
			const FString RelativePath = FAGX_EditorUtilities::GetRelativePath(
				FPaths::GetPath(RootDir), Material->GetPathName());

			Component->UnrealMaterialToImportGuid.Add(RelativePath, Guid);
		}

		return EAGX_ImportResult::Success;
	}
}

UBlueprint* FAGX_ImporterToEditor::Import(const FAGX_ImporterSettings& Settings)
{
	using namespace AGX_ImporterToEditor_helpers;
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return nullptr;

	ModelName = AGX_ImporterToEditor_helpers::MakeModelName(Result.Actor->GetName());
	RootDirectory = MakeRootDirectoryPath(ModelName);

	if (!ValidateImportEnum(FinalizeModelSourceComponent(*Result.Context, RootDirectory)))
		return nullptr;

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

	if (!ValidateBlueprintForReimport(BaseBP))
		return false;

	if (GEditor != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();

	// As a safety measure, we save and compile the OpenBlueprint (now closed) so that all
	// references according to the state on disk are up to date.
	if (OpenBlueprint != nullptr)
		FAGX_EditorUtilities::SaveAndCompile(*OpenBlueprint);

	PreReimportSetup();
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	if (!ValidateImportResult(Result, Settings))
		return false;

	RootDirectory = GetModelDirectoryPathFromBaseBlueprint(BaseBP);

	if (!ValidateImportEnum(FinalizeModelSourceComponent(*Result.Context, RootDirectory)))
		return false;

	const auto UpdateResult = UpdateBlueprint(BaseBP, Importer.GetContext());
	if (!ValidateImportEnum(UpdateResult))
		return false;

	if (Settings.bOpenBlueprintEditorAfterImport && OpenBlueprint != nullptr)
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(OpenBlueprint);

	return true;
}

template <typename T>
T* FAGX_ImporterToEditor::UpdateOrCreateAsset(T& Source, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	const FString AssetType = GetAssetTypeFromType<T>(Source);
	const FString DirPath = FPaths::Combine(RootDirectory, AssetType);
	T* Asset = nullptr;
	if constexpr (HasImportGuid<T>::value)
		Asset = FAGX_EditorUtilities::FindAsset<T>(Source.ImportGuid, DirPath);
	else
		Asset = FAGX_EditorUtilities::FindAsset<T>(Source.GetName(), DirPath);

	if (Asset == nullptr)
	{
		WriteAssetToDisk(RootDirectory, AssetType, Source, Context.SessionGuid);
		return &Source; // We are done.
	}

	// At this point we have identified an existing Asset that we need to update using Source.

	if constexpr (std::is_same_v<T, UStaticMesh>)
	{
		// UStaticMesh needs some special handling, because it is a complicated object which also
		// can take a lot of time creating/building. If the new Static Mesh and the old one is
		// "equal", then we do nothing except add an entry to the TransientToAsset map so that we
		// never copy the new one over to a component during reimport. If they are not equal, we
		// update the old one, just like for any other assets during reimport.
		if (!AGX_MeshUtilities::AreStaticMeshesEqual(&Source, Asset))
		{
			bool Result = AGX_MeshUtilities::CopyStaticMesh(&Source, Asset);
			AGX_CHECK(Result);
			AddSessionGuid(Context.SessionGuid, *Asset);
			Result = FAGX_ObjectUtilities::SaveAsset(*Asset);
			AGX_CHECK(Result);
		}
	}
	else // Non Static Mesh Asset.
	{
		// For shared assets, we might be copying and saving multiple times here, but we assume
		// these operations are relatively cheap, and keep the code simple here.
		bool Result = FAGX_ObjectUtilities::CopyProperties(Source, *Asset, false);
		AGX_CHECK(Result);
		AddSessionGuid(Context.SessionGuid, *Asset);
		Result = FAGX_ObjectUtilities::SaveAsset(*Asset);
		AGX_CHECK(Result);
	}

	TransientToAsset.Add(&Source, Asset);
	return Asset;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateBlueprint(
	UBlueprint& Blueprint, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	EAGX_ImportResult Result = UpdateAssets(Blueprint, Context);
	if (IsUnrecoverableError(Result))
		return Result;

	Result |= UpdateComponents(Blueprint, Context);
	if (IsUnrecoverableError(Result))
		return Result;

	RemoveDeletedComponents(Blueprint, Context.SessionGuid);
	RemoveDeletedAssets(RootDirectory, Context.SessionGuid);

	return Result;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateAssets(
	UBlueprint& Blueprint, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;
	EAGX_ImportResult Result = EAGX_ImportResult::Success;

	if (Context.MSThresholds != nullptr)
	{
		for (const auto& [Guid, MST] : *Context.MSThresholds)
		{
			const auto A = UpdateOrCreateAsset(*MST, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.RenderMaterials != nullptr)
	{
		for (const auto& [Guid, Rm] : *Context.RenderMaterials)
		{
			const auto A = UpdateOrCreateAsset(*Rm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.CollisionStaticMeshes != nullptr)
	{
		for (const auto& [Guid, Sm] : *Context.CollisionStaticMeshes)
		{
			const auto A = UpdateOrCreateAsset(*Sm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	if (Context.RenderStaticMeshes != nullptr)
	{
		for (const auto& [Guid, Sm] : *Context.RenderStaticMeshes)
		{
			const auto A = UpdateOrCreateAsset(*Sm, Context);
			AGX_CHECK(A != nullptr);
			if (A == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
		}
	}

	return Result;
}

EAGX_ImportResult FAGX_ImporterToEditor::UpdateComponents(
	UBlueprint& Blueprint, const FAGX_ImportContext& Context)
{
	using namespace AGX_ImporterToEditor_helpers;

	// The order of the Components below must be such that parent Components are updated first, and
	// potential child Components last. This is because the children Components may be assigned a
	// new parent during reimport and to avoid errors, the parent should then be up to date so that
	// it can be correctly identified (for example in GetCorrespondingAttachParent above).

	FAGX_SCSNodeCollection Nodes(Blueprint);
	WriteImportTag(*Nodes.RootComponent->ComponentTemplate, Context.SessionGuid);
	EAGX_ImportResult Result = EAGX_ImportResult::Success;

	if (Context.RigidBodies != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.RigidBodies)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.RigidBodies, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset);
		}
	}

	if (auto Component = Context.ModelSourceComponent)
	{
		CopyProperties(
			*Component, *Nodes.ModelSourceComponent->ComponentTemplate, TransientToAsset);
	}

	if (Context.Shapes != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.Shapes)
		{
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Nodes.Shapes, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset);
		}
	}

	if (Context.CollisionStaticMeshCom != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.CollisionStaticMeshCom)
		{
			TMap<FGuid, USCS_Node*> Unused;
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Unused, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset);
		}
	}

	if (Context.RenderStaticMeshCom != nullptr)
	{
		for (const auto& [Guid, Component] : *Context.RenderStaticMeshCom)
		{
			TMap<FGuid, USCS_Node*> Unused;
			USCS_Node* N = GetOrCreateNode(Guid, *Component, Nodes, Unused, Blueprint);
			if (N == nullptr)
				Result |= EAGX_ImportResult::RecoverableErrorsOccured;
			else
				CopyProperties(*Component, *N->ComponentTemplate, TransientToAsset);
		}
	}

	return Result;
}
