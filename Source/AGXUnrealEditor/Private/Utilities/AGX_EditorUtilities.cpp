#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Editor/EditorEngine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameEngine.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Char.h"
#include "Misc/MessageDialog.h"
#include "Misc/EngineVersionComparison.h"
#include "RawMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

// AGXUnreal includes.
#include "AGX_BoxShapeComponent.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_TrimeshShapeComponent.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintFrameActor.h"

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

std::tuple<AActor*, USceneComponent*> FAGX_EditorUtilities::CreateEmptyActor(
	const FTransform& Transform, UWorld* World)
{
	/// \todo The intention is to mimmic dragging in an "Empty Actor" from the
	/// Place mode. Investigate if we can use ActorFactoryEmptyActor instead.

	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (NewActor == nullptr)
	{
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
	TShapeComponent* CreateShapeComponent(AActor* Owner, USceneComponent* Outer)
	{
		/// \todo Is the Owner pointless here since we do `AttachToComponent`
		/// immediately afterwards?
		UClass* Class = TShapeComponent::StaticClass();
		TShapeComponent* Shape = NewObject<TShapeComponent>(Owner, Class);
		Owner->AddInstanceComponent(Shape);
		Shape->RegisterComponent();
		const bool Attached = Shape->AttachToComponent(
			Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		check(Attached);
		return Shape;
	}
}

UAGX_RigidBodyComponent* FAGX_EditorUtilities::CreateRigidBody(AActor* Owner)
{
	UAGX_RigidBodyComponent* Body = ::CreateComponent<UAGX_RigidBodyComponent>(Owner);
	Body->AttachToComponent(
		Owner->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
	Body->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	Body->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
	return Body;
}

UAGX_SphereShapeComponent* FAGX_EditorUtilities::CreateSphereShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_SphereShapeComponent>(Owner, Outer);
}

UAGX_BoxShapeComponent* FAGX_EditorUtilities::CreateBoxShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_BoxShapeComponent>(Owner, Outer);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer);
}

namespace
{
	FString SanitizeName(const FString& Name)
	{
		FString Sanitized;
		Sanitized.Reserve(Name.Len());
		for (TCHAR C : Name)
		{
			if (TChar<TCHAR>::IsAlnum(C))
			{
				Sanitized.AppendChar(C);
			}
		}
		return Sanitized;
	}

	void AddRawMeshToStaticMesh(FRawMesh& RawMesh, UStaticMesh* StaticMesh)
	{
		StaticMesh->StaticMaterials.Add(FStaticMaterial());
#if UE_VERSION_OLDER_THAN(4, 23, 0)
		StaticMesh->SourceModels.Emplace();
		FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
#else
		StaticMesh->GetSourceModels().Emplace();
		FStaticMeshSourceModel& SourceModel = StaticMesh->GetSourceModels().Last();
#endif
		SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
		FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;

		// Somewhat unclear what all these should be.
		BuildSettings.bRecomputeNormals = true;
		BuildSettings.bRecomputeTangents = true;
		BuildSettings.bUseMikkTSpace = false; /// \todo Why not? MikkTSpace seems to be a standard.
		BuildSettings.bGenerateLightmapUVs = true;
		BuildSettings.bBuildAdjacencyBuffer = false;
		BuildSettings.bBuildReversedIndexBuffer = false;
		BuildSettings.bUseFullPrecisionUVs = false;
		BuildSettings.bUseHighPrecisionTangentBasis = false;
	}

	struct FAssetId
	{
		FString PackagePath;
		FString AssetName;

		bool IsValid() const
		{
			return !PackagePath.IsEmpty() && !AssetName.IsEmpty();
		}
	};

	FAssetId CreateTrimeshAsset(FRawMesh& RawMesh, const FString& MeshName)
	{
		FString PackagePath = FString(TEXT("/Game/ImportedAGXMeshes/"));
		FString AssetName = MeshName;

		IAssetTools& AssetTools =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.CreateUniqueAssetName(PackagePath, AssetName, PackagePath, AssetName);

		UPackage* Package = CreatePackage(nullptr, *PackagePath);
		UPackage* OuterPackage = Package->GetOutermost(); /// \todo Why do we need this?
		Package->FullyLoad(); /// \todo Is this required or not?
		OuterPackage->FullyLoad(); /// \todo If required, on which package?
		UStaticMesh* StaticMesh = /// \todo What RF_ flags should be used?
			NewObject<UStaticMesh>(OuterPackage, FName(*AssetName), RF_Standalone | RF_Public);

		AddRawMeshToStaticMesh(RawMesh, StaticMesh);

		StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
		StaticMesh->CreateBodySetup(); /// \todo Not sure what these two
		StaticMesh->SetLightingGuid(); /// does, or if they are needed.

		FAssetRegistryModule::AssetCreated(StaticMesh);
		StaticMesh->MarkPackageDirty();
		StaticMesh->PostEditChange();
		StaticMesh->AddToRoot();
		Package->SetDirtyFlag(true);

		FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
		if (PackageFilename.IsEmpty())
		{
			/// \todo What should we do here, other than log and bail?
			UE_LOG(
				LogAGX, Error,
				TEXT("Unreal Engine unable to provide package filename for package path '%s'."),
				*PackagePath);
			return FAssetId();
		}
		bool bSaved = UPackage::SavePackage(
			Package, StaticMesh, RF_Public | RF_Standalone | RF_MarkAsRootSet, *PackageFilename);
		if (!bSaved)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unreal Engine unable to save package '%s' to file '%s'."),
				*PackagePath, *PackageFilename);
			return FAssetId();
		}

		/// \todo What should be returned here? What does ConstructorHelpers::FObjectFinder expect?
		/// Should it be the `PackagePath`, i.e. '/Game/ImportedAGXMeshes/mesh1', or a full
		/// reference, i.e., '/Game/ImportedAGXMeshes/mesh1.mesh1', created as 'PackagePath + . +
		/// AssetName'?
		return {PackagePath, AssetName};
	}

}

UStaticMeshComponent* FAGX_EditorUtilities::CreateStaticMesh(
	AActor* Owner, UAGX_TrimeshShapeComponent* Outer, const FTrimeshShapeBarrier& Trimesh)
{
	FRawMesh RawMesh = Trimesh.GetRawMesh();
	FString TrimeshName = SanitizeName(Trimesh.GetSourceName());
	if (TrimeshName.IsEmpty())
	{
		TrimeshName = TEXT("ImportedAGXMesh");
	}

	FAssetId AssetId = CreateTrimeshAsset(RawMesh, TrimeshName);
	if (!AssetId.IsValid())
	{
		/// \todo What should we do here, other than giving up?
		// No need to log, should have been done by CreateTrimeshAsset.
		return nullptr; /// \todo Don't return nullptr, return an empty UStaticMeshComponent.
	}

	UE_LOG(
		LogAGX, Log, TEXT("Trimesh '%s' successfully stored to package '%s', asset '%s'"),
		*Trimesh.GetSourceName(), *AssetId.PackagePath, *AssetId.AssetName);

	FString MeshAssetPath =
		FString::Printf(TEXT("%s.%s"), *AssetId.PackagePath, *AssetId.AssetName);
	UE_LOG(LogAGX, Log, TEXT("Loading imported mesh asset from '%s'."), *MeshAssetPath);
	UStaticMesh* MeshAsset = LoadObject<UStaticMesh>(NULL, *MeshAssetPath, NULL, LOAD_None, NULL);
	if (MeshAsset == nullptr)
	{
		/// \todo What should we do here, other than printing a message and giving up?
		UE_LOG(LogAGX, Error, TEXT("Failed to load imported mesh asset '%s'"), *MeshAssetPath);
		return nullptr; /// \todo Don't reeturn nullptr, return an empty UStaticMeshComponent.
	}

	UStaticMeshComponent* StaticMeshComponent =
		NewObject<UStaticMeshComponent>(Outer, FName(*AssetId.AssetName)); // Which flags?
	StaticMeshComponent->SetStaticMesh(MeshAsset);
	Owner->AddInstanceComponent(StaticMeshComponent);
	StaticMeshComponent->RegisterComponent();
	const bool Attached = StaticMeshComponent->AttachToComponent(
		Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (!Attached)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to attach imported StaticMeshComponent '%s' to Actor '%s'."), *TrimeshName,
			*Owner->GetName());
	}
	return StaticMeshComponent;
}

AAGX_Constraint* FAGX_EditorUtilities::CreateConstraint(
	UClass* ConstraintType, AActor* RigidBody1, AActor* RigidBody2, bool bInPlayingWorldIfAvailable,
	bool bSelect, bool bShowNotification)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();

	check(World);
	check(ConstraintType->IsChildOf<AAGX_Constraint>());

	// Create the new Constraint Actor.
	AAGX_Constraint* NewActor =
		World->SpawnActorDeferred<AAGX_Constraint>(ConstraintType, FTransform::Identity);

	check(NewActor);

	NewActor->BodyAttachment1.RigidBodyActor = RigidBody1;
	NewActor->BodyAttachment2.RigidBodyActor = RigidBody2;

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
	AActor* ParentRigidBody, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();
	check(World);

	// Create the new Constraint Frame Actor.
	AAGX_ConstraintFrameActor* NewActor = World->SpawnActor<AAGX_ConstraintFrameActor>();
	check(NewActor);

	// Set the new actor as child to the Rigid Body.
	if (ParentRigidBody)
	{
		if (ParentRigidBody->GetWorld() == World)
		{
			NewActor->AttachToActor(
				ParentRigidBody, FAttachmentTransformRules::KeepRelativeTransform);
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
	Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
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

void FAGX_EditorUtilities::ShowDialogBox(const FText& Text)
{
#if 0
	// Example of how an FText can be created.
	FText DialogText = FText::Format(LOCTEXT("PluginButtonDialogText", "{0} was recompiled at {1}.\n{2}"),
		FText::FromString(TEXT(__FILE__)), FText::FromString(TEXT(__TIME__)),
		FText::FromString(TEXT("Create body before root component.")));
#endif
	FMessageDialog::Open(EAppMsgType::Ok, Text);
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
	auto AssignOutActors = [OutActor1, OutActor2](AActor* RigidBodyActor) {
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
			if (UAGX_RigidBodyComponent::GetFromActor(SelectedActor))
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
		if (SubtreeRoot != IgnoreActor && UAGX_RigidBodyComponent::GetFromActor(SubtreeRoot))
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
		if (Actor != IgnoreActor && UAGX_RigidBodyComponent::GetFromActor(Actor))
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

#undef LOCTEXT_NAMESPACE
