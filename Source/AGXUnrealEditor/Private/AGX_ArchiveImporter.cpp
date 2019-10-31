#include "AGX_ArchiveImporter.h"
#include "AGXArchiveReader.h"
#include "AGX_EditorUtilities.h"
#include "RigidBodyBarrier.h"

#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_BoxShapeComponent.h"

#include "Math/Transform.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

#include <iostream>

namespace
{
	/// \todo Investigate if we can use ActorFactoryEmptyActor here.
	std::tuple<AActor*, USceneComponent*> SpawnEmptyActor(const FTransform& Transform, UWorld* World)
	{
		AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), Transform);
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

		return {NewActor, Root};
	}

	template <typename TShapeFactory>
	AActor* InstantiateBody(const FRigidBodyBarrier* Body, UWorld* World, TShapeFactory ShapeFactory)
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX sphere body with name '%s'."), *Body->GetName());

		/// \todo Consider using the state synchronization functions we already
		/// have, the ones used between time steps.

		AActor* NewActor;
		USceneComponent* Root;
		FTransform Transform(Body->GetRotation(), Body->GetPosition(World));
		std::tie(NewActor, Root) = SpawnEmptyActor(Transform, World);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s'."), *Body->GetName());
			return nullptr;
		}
		if (Root == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create SceneComponent for body '%s'."), *Body->GetName());
			return nullptr;
		}

		NewActor->SetActorLabel(Body->GetName());

		/// \todo For some reason the actor location must be set again after
		/// creating the root SceneComponent, or else the Actor remain at the
		/// origin. I'm assuming we must set rotation as well, but haven't
		/// tested yet.
		NewActor->SetActorLocation(Body->GetPosition(World));

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		if (NewBody == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create AGX RigidBodyComponent for body '%s'."), *Body->GetName());
			/// \todo Do we need to destroy the Actor and the RigidBodyComopnent here?
			return nullptr;
		}

		NewBody->Rename(TEXT("AGX_RigidBodyComponent"));
		NewBody->Mass = Body->GetMass();
		NewBody->MotionControl = Body->GetMotionControl();

		ShapeFactory(NewActor, Root);

		return NewActor;
	}
}

AActor* AGX_ArchiveImporter::ImportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	check(World);

	std::cout << "C++ version: " << __cplusplus << '\n';

	FAGXArchiveReader Archive(ArchivePath);
	if (Archive.GetBoxBodies().Num() == 0 && Archive.GetSphereBodies().Num() == 0)
	{
		/// \todo Proper error handling.
		FAGX_EditorUtilities::ShowNotification(
			LOCTEXT("No bodies", "No bodies found in .agx archive. Perhaps read failure."));
		return nullptr;
	}

	/// \todo Consider placing the ImportedRoot at the center of the imported bodies.
	AActor* ImportRoot;
	USceneComponent* Root;
	std::tie(ImportRoot, Root) = ::SpawnEmptyActor(FTransform::Identity, World); // World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);

	for (auto& BoxBody : Archive.GetBoxBodies())
	{
		AActor* NewActor =
			::InstantiateBody(BoxBody.Body, World, [&BoxBody, World](AActor* NewActor, USceneComponent* Root) {
				UAGX_BoxShapeComponent* NewBox = FAGX_EditorUtilities::CreateBoxShape(NewActor, Root);
				NewBox->HalfExtent = BoxBody.Box->GetHalfExtents(World);
			});

		NewActor->AttachToActor(ImportRoot, FAttachmentTransformRules::KeepWorldTransform);
	}

	for (auto& SphereBody : Archive.GetSphereBodies())
	{
		AActor* NewActor =
			::InstantiateBody(SphereBody.Body, World, [&SphereBody, World](AActor* NewActor, USceneComponent* Root) {
				const FSphereShapeBarrier* Sphere = SphereBody.Sphere;
				UAGX_SphereShapeComponent* NewSphere = FAGX_EditorUtilities::CreateSphereShape(NewActor, Root);
				NewSphere->Radius = Sphere->GetRadius(World);
			});

		NewActor->AttachToActor(ImportRoot, FAttachmentTransformRules::KeepWorldTransform);
	}

	return ImportRoot;
}

#undef LOCTEXT_NAMESPACE
