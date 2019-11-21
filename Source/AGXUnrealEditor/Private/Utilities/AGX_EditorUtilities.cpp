#include "Utilities/AGX_EditorUtilities.h"

#include "Editor/EditorEngine.h"
#include "Engine/GameEngine.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFramework/PlayerController.h"
#include "Misc/MessageDialog.h"
#include "Misc/Char.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "AssetRegistryModule.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "RawMesh.h"

#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_BoxShapeComponent.h"
#include "AGX_TrimeshShapeComponent.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintFrameActor.h"

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

std::tuple<AActor*, USceneComponent*> FAGX_EditorUtilities::CreateEmptyActor(const FTransform& Transform, UWorld* World)
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
	USceneComponent* Root =
		NewObject<USceneComponent>(NewActor, USceneComponent::GetDefaultSceneRootVariableName() /*, RF_Transactional*/);
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
			UE_LOG(LogTemp, Log, TEXT("Could not create component %s."), *Class->GetName());
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
		const bool Attached = Shape->AttachToComponent(Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		check(Attached);
		return Shape;
	}
}

UAGX_RigidBodyComponent* FAGX_EditorUtilities::CreateRigidBody(AActor* Owner)
{
	return ::CreateComponent<UAGX_RigidBodyComponent>(Owner);
}

UAGX_SphereShapeComponent* FAGX_EditorUtilities::CreateSphereShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_SphereShapeComponent>(Owner, Outer);
}

UAGX_BoxShapeComponent* FAGX_EditorUtilities::CreateBoxShape(AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_BoxShapeComponent>(Owner, Outer);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(AActor* Owner, USceneComponent* Outer)
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

	FString PackagePath = TEXT("/Game/ImportedAGXMeshes");
	UPackage* Package = CreatePackage(nullptr, *PackagePath);
	Package->FullyLoad();

	FName UniqueMeshName = MakeUniqueObjectName(Package, UStaticMesh::StaticClass(), *TrimeshName);
	UStaticMesh* StaticMesh =
		NewObject<UStaticMesh>(Package, UniqueMeshName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	StaticMesh->StaticMaterials.Add(FStaticMaterial());
	StaticMesh->SourceModels.Emplace();
	FStaticMeshSourceModel& SourceModel = StaticMesh->SourceModels.Last();
	SourceModel.RawMeshBulkData->SaveRawMesh(RawMesh);
	FMeshBuildSettings& BuildSettings = SourceModel.BuildSettings;
	// Somewhat unclear what all these should be.
	BuildSettings.bRecomputeNormals = true;
	BuildSettings.bRecomputeTangents = true;
	BuildSettings.bUseMikkTSpace = false;
	BuildSettings.bGenerateLightmapUVs = true;
	BuildSettings.bBuildAdjacencyBuffer = false;
	BuildSettings.bBuildReversedIndexBuffer = false;
	BuildSettings.bUseFullPrecisionUVs = false;
	BuildSettings.bUseHighPrecisionTangentBasis = false;

	StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
	StaticMesh->CreateBodySetup();
	StaticMesh->SetLightingGuid();
	StaticMesh->PostEditChange();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(StaticMesh);
	FString AssetFileName = FPackageName::LongPackageNameToFilename(
		PackagePath + "/" + UniqueMeshName.ToString(), FPackageName::GetAssetPackageExtension());
	if (AssetFileName.IsEmpty())
	{
		AssetFileName = FPaths::ProjectContentDir() + "/FallbackFilename.uasset";
		UE_LOG(LogTemp, Warning, TEXT("Package path '%s' produced empty long package name."), *PackagePath);
		UE_LOG(LogTemp, Warning, TEXT("Using fallback name '%s'."), *AssetFileName);
	}
	bool bSaved = UPackage::SavePackage(Package, StaticMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
		*AssetFileName, GError, nullptr, true, true, SAVE_NoError);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("Save of imported StaticMesh asset failed."));
		// No return intentional. We want to create a UStaticMeshComponent for
		// the StaticMesh even if it couldn't be saved to disk.
	}

	UClass* Class = UStaticMeshComponent::StaticClass();
	UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(Outer, Class, UniqueMeshName);
	StaticMeshComponent->SetStaticMesh(StaticMesh);
	Owner->AddInstanceComponent(StaticMeshComponent);
	StaticMeshComponent->RegisterComponent();
	const bool Attached =
		StaticMeshComponent->AttachToComponent(Outer, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	check(Attached);
	return StaticMeshComponent;
}

AAGX_Constraint* FAGX_EditorUtilities::CreateConstraint(UClass* ConstraintType, AActor* RigidBody1, AActor* RigidBody2,
	bool bInPlayingWorldIfAvailable, bool bSelect, bool bShowNotification)
{
	UWorld* World = bInPlayingWorldIfAvailable ? GetCurrentWorld() : GetEditorWorld();

	check(World);
	check(ConstraintType->IsChildOf<AAGX_Constraint>());

	// Create the new Constraint Actor.
	AAGX_Constraint* NewActor = World->SpawnActorDeferred<AAGX_Constraint>(ConstraintType, FTransform::Identity);

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
			NewActor->AttachToActor(ParentRigidBody, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			UE_LOG(LogTemp, Log,
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
		ShowNotification(LOCTEXT("CreateConstraintFrameActorSucceded", "AGX Constraint Frame Actor Created"));
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
		GEditor->SelectActor(Actor,
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

void FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(AActor** OutActor1, AActor** OutActor2,
	bool bSearchSubtrees, bool bSearchAncestors)
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
	auto AssignOutActors = [OutActor1, OutActor2](AActor *RigidBodyActor)
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
				AActor* RigidBodyActor = GetRigidBodyActorFromSubtree(SelectedActor,
					(OutActor1 ? *OutActor1 : nullptr));

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
				AActor* RigidBodyActor = GetRigidBodyActorFromAncestors(SelectedActor,
					(OutActor1 ? *OutActor1 : nullptr));

				// Found one. Assign it to next available OutActor!
				if (!AssignOutActors(RigidBodyActor))
				{
					return; // return if no more available OutActors
				}
			}
		}
	}
}

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromSubtree(AActor* SubtreeRoot, const AActor* IgnoreActor)
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

AActor* FAGX_EditorUtilities::GetRigidBodyActorFromAncestors(AActor* Actor, const AActor* IgnoreActor)
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
			RigidBodyActor = GetRigidBodyActorFromAncestors(Actor->GetAttachParentActor(), IgnoreActor);
		}
	}

	return RigidBodyActor;
}

void FAGX_EditorUtilities::GetAllClassesOfType(TArray<UClass*>& OutMatches, UClass* BaseClass, bool bIncludeAbstract)
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
