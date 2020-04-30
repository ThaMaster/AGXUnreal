#include "Utilities/AGX_EditorUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"

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

#define LOCTEXT_NAMESPACE "FAGX_EditorUtilities"

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
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	Body->RelativeLocation = FVector(0.0f, 0.0f, 0.0f);
	Body->RelativeRotation = FRotator(0.0f, 0.0f, 0.0f);
#else
	Body->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Body->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
#endif
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

UAGX_CylinderShapeComponent* FAGX_EditorUtilities::CreateCylinderShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_CylinderShapeComponent>(Owner, Outer);
}

UAGX_TrimeshShapeComponent* FAGX_EditorUtilities::CreateTrimeshShape(
	AActor* Owner, USceneComponent* Outer)
{
	return ::CreateShapeComponent<UAGX_TrimeshShapeComponent>(Owner, Outer);
}

namespace
{
	/**
	 * Remove characters that are unsafe to use in object names, content
	 * references or filenames. Unsupported characters are dropped, so check the
	 * returned string for emptyness and have a fallback.
	 *
	 * May remove more characters than necessary.
	 *
	 * @param Name The name to sanitize.
	 * @return The name with all dangerous characters removed.
	 */
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

	FString CreateAssetName(FString TrimeshSourceName, FString ActorName)
	{
		TrimeshSourceName = SanitizeName(TrimeshSourceName);
		if (!TrimeshSourceName.IsEmpty())
		{
			return TrimeshSourceName;
		}

		ActorName = SanitizeName(ActorName);
		if (!ActorName.IsEmpty())
		{
			return ActorName;
		}

		return TEXT("ImportedAGXMesh");
	}

	/**
	 * Apply the RawMesh data to the StaticMesh.
	 *
	 * @param RawMesh - The RawMesh holding the mesh data.
	 * @param StaticMesh - The StaticMesh that should receive the mesh data.
	 */
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
		BuildSettings.bUseMikkTSpace = true;
		BuildSettings.bGenerateLightmapUVs = true;
		BuildSettings.bBuildAdjacencyBuffer = false;
		BuildSettings.bBuildReversedIndexBuffer = false;
		BuildSettings.bUseFullPrecisionUVs = false;
		BuildSettings.bUseHighPrecisionTangentBasis = false;
	}

	/**
	 * A way to identify an asset within the project content.
	 *
	 * Contains the full package path to the asset as well as the asset name.
	 * ex:
	 * 	PackagePath: "/Game/ImportedAGXMeshes/ImportedAGXMesh4"
	 * 	AssetName: "ImportedAGXMesh4"
	 */
	struct FAssetId
	{
		FString PackagePath;
		FString AssetName;

		bool IsValid() const
		{
			return !PackagePath.IsEmpty() && !AssetName.IsEmpty();
		}
	};

	/**
	 * Convert a raw mesh into a StaticMesh asset on disk.
	 *
	 * The mesh asset is stored to the /Game/ImportedAGXMeshes/<AssetFolderName>
	 * folder. The given MeshName will only be used as-is if it doesn't collide
	 * with an already existing asset. The final name and the full asset path is
	 * returned.
	 *
	 * @param RawMesh - The mesh to store as an asset.
	 * @param AssetFolderName - The folder within '/Game/ImportedAGXMeshes/' in which the asset
	 * should be stored.
	 * @param MeshName - The base name to give the asset. A unique name based on the base name will
	 * become the final name.
	 * @return The path to the created package and the final name of the asset.
	 */
	FAssetId CreateTrimeshAsset(
		FRawMesh& RawMesh, const FString& AssetFolderName, const FString& MeshName)
	{
		// Find actual package path and a unique asset name.
		FString PackagePath =
			FString::Printf(TEXT("/Game/ImportedAGXMeshes/%s/"), *AssetFolderName);
		FString AssetName = MeshName;
		IAssetTools& AssetTools =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.CreateUniqueAssetName(PackagePath, AssetName, PackagePath, AssetName);

		// Create the package that will hold our mesh asset.
		UPackage* Package = CreatePackage(nullptr, *PackagePath);
#if 0
		/// \todo Unclear if this is needed or not. Leaving it out for now but
		/// test with it restored if there are problems.
		Package->FullyLoad();
#endif

		// Create the actual mesh object and fill it with our mesh data.
		UStaticMesh* StaticMesh =
			NewObject<UStaticMesh>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		AddRawMeshToStaticMesh(RawMesh, StaticMesh);

		// Package is now complete. Trigger all the "engine stuff" to finalize it.
		StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
		FAssetRegistryModule::AssetCreated(StaticMesh);
		StaticMesh->MarkPackageDirty();
		StaticMesh->PostEditChange();
		StaticMesh->AddToRoot();
		Package->SetDirtyFlag(true);

		// Store our new package to disk.
		FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());
		if (PackageFilename.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unreal Engine unable to provide a package filename for package path '%s'."),
				*PackagePath);
			return FAssetId();
		}
		bool bSaved = UPackage::SavePackage(Package, StaticMesh, RF_NoFlags, *PackageFilename);
		if (!bSaved)
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unreal Engine unable to save package '%s' to file '%s'."),
				*PackagePath, *PackageFilename);
			return FAssetId();
		}

		return {PackagePath, AssetName};
	}

	/**
	 * Creates a FRawMesh from an FTrimeshShapeBarrier.
	 *
	 * @param Trimesh - The trimesh barrier used to create the raw mesh.
	 * @return A raw mesh created from the trimesh barrier passed as argument.
	 */
	FRawMesh CreateRawMeshFromTrimesh(const FTrimeshShapeBarrier& Trimesh)
	{
		FRawMesh RawMesh;

		RawMesh.VertexPositions = Trimesh.GetVertexPositions();
		RawMesh.WedgeIndices = Trimesh.GetVertexIndices();
		const TArray<FVector> TriangleNormals = Trimesh.GetTriangleNormals();

		const int32 NumFaces = TriangleNormals.Num();
		const int32 NumWedges = RawMesh.WedgeIndices.Num();
		check(NumWedges == 3 * NumFaces);

		RawMesh.FaceMaterialIndices.Reserve(NumFaces);
		RawMesh.FaceSmoothingMasks.Reserve(NumFaces);
		for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
		{
			RawMesh.FaceMaterialIndices.Add(0);
			RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
		}

		RawMesh.WedgeTangentX.Reserve(NumWedges);
		RawMesh.WedgeTangentY.Reserve(NumWedges);
		RawMesh.WedgeTangentZ.Reserve(NumWedges);
		RawMesh.WedgeColors.Reserve(NumWedges);
		for (int32 UVIndex = 0; UVIndex < MAX_MESH_TEXTURE_COORDS; ++UVIndex)
		{
			RawMesh.WedgeTexCoords[UVIndex].Reserve(NumWedges);
		}

		for (int32 i = 0; i < NumWedges; ++i)
		{
			RawMesh.WedgeTangentX.Add(FVector(0.0f, 0.0f, 0.0f));
			RawMesh.WedgeTangentY.Add(FVector(0.0f, 0.0f, 0.0f));
			RawMesh.WedgeColors.Add(FColor(255, 255, 255));
		}

		// Apply texture coordinates.
		const TArray<FVector2D> TextureCoordiantes = Trimesh.GetRenderDataTextureCoordinates();
		const TArray<uint32> RenderDataIndices = Trimesh.GetRenderDataIndices();
		const bool TextureCoordinatesUsable = RenderDataIndices.Num() == NumWedges;
		if (!TextureCoordinatesUsable)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot use render data for trimesh '%s' because the number of"
					 "render data indices (%d) does not match the number of wedges (%d)."),
				*Trimesh.GetSourceName(), RenderDataIndices.Num(), NumWedges);
		}

		for (int32 i = 0; i < NumWedges; ++i)
		{
			for (int32 UVIndex = 0; UVIndex < MAX_MESH_TEXTURE_COORDS; ++UVIndex)
			{
				if (TextureCoordinatesUsable)
				{
					RawMesh.WedgeTexCoords[UVIndex].Add(TextureCoordiantes[RenderDataIndices[i]]);
				}
				else
				{
					// If we didn't get usable texture coordinates, set them to zero. Not setting
					// them to anything will result in a runtime crash.
					RawMesh.WedgeTexCoords[UVIndex].Add(FVector2D(0.0f, 0.0f));
				}
			}
		}

		for (const FVector& TriangleNormal : TriangleNormals)
		{
			// The native trimesh store normals per triangle and not per vertex as
			// Unreal want it. Duplicating the normals here, but if we want smooth
			// shading then we should try to let Unreal compute the normals for us.
			RawMesh.WedgeTangentZ.Add(TriangleNormal);
			RawMesh.WedgeTangentZ.Add(TriangleNormal);
			RawMesh.WedgeTangentZ.Add(TriangleNormal);
		}

		return RawMesh;
	}
}

UStaticMeshComponent* FAGX_EditorUtilities::CreateStaticMeshAsset(
	AActor* Owner, UAGX_TrimeshShapeComponent* Outer, const FTrimeshShapeBarrier& Trimesh,
	const FString& AssetFolderName)
{
	FRawMesh RawMesh = CreateRawMeshFromTrimesh(Trimesh);
	FString TrimeshName = CreateAssetName(Trimesh.GetSourceName(), Owner->GetActorLabel());
	FAssetId AssetId = CreateTrimeshAsset(RawMesh, AssetFolderName, TrimeshName);
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

	/// \todo Which EObjectFlags should be passed to NewObject?
	UStaticMeshComponent* StaticMeshComponent =
		NewObject<UStaticMeshComponent>(Outer, FName(*AssetId.AssetName));
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

UAGX_ConstraintComponent* FAGX_EditorUtilities::CreateConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* RigidBody1, UAGX_RigidBodyComponent* RigidBody2,
	UClass* ConstraintType)
{
	UAGX_ConstraintComponent* Constraint =
		NewObject<UAGX_ConstraintComponent>(Owner, ConstraintType);
	Owner->AddInstanceComponent(Constraint);
	Constraint->RegisterComponent();
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
	return Constraint;
}

UAGX_HingeConstraintComponent* FAGX_EditorUtilities::CreateHingeConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2)
{
	return CreateConstraintComponent<UAGX_HingeConstraintComponent>(Owner, Body1, Body2);
}

UAGX_PrismaticConstraintComponent* FAGX_EditorUtilities::CreatePrismaticConstraintComponent(
	AActor* Owner, UAGX_RigidBodyComponent* Body1, UAGX_RigidBodyComponent* Body2)
{
	return CreateConstraintComponent<UAGX_PrismaticConstraintComponent>(Owner, Body1, Body2);
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

#undef LOCTEXT_NAMESPACE
