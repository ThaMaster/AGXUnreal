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

AActor* AGX_ArchiveImporter::ImportAGXArchive(const FString& ArchivePath)
{
	UClass* ActorClass = AActor::StaticClass();
	FName RootName = USceneComponent::GetDefaultSceneRootVariableName();
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
	AActor* ImportRoot = World->SpawnActor<AActor>(ActorClass, FTransform::Identity);

	for (auto& BoxBody : Archive.GetBoxBodies())
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX box body with name %s at %f."), *BoxBody.Body->GetName(),
			BoxBody.Body->GetPosition(World).X);

		const FRigidBodyBarrier* Body = BoxBody.Body;
		const FBoxShapeBarrier* Box = BoxBody.Box;

		/// \todo Consider using the state synchronization functions we already
		/// have, the ones used between time steps.

		FTransform Transform(Body->GetRotation(), Body->GetPosition(World));
		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Transform);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s."), *Body->GetName());
			continue;
		}
		NewActor->SetActorLabel(Body->GetName());

		/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
		/// Related to undo/redo, I think.
		USceneComponent* Root = NewObject<USceneComponent>(NewActor, RootName /*, RF_Transactional*/);
		NewActor->SetRootComponent(Root);
		NewActor->AddInstanceComponent(Root);
		Root->RegisterComponent();

		/// \todo For some reason the actor location must be set again after
		/// creating the root SceneComponent, or else the Actor remain at the
		/// origin. I'm assuming we must set rotation as well, but haven't
		/// tested yet.
		NewActor->SetActorLocation(Body->GetPosition(World));

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		if (NewBody == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create AGX RigidBody for %s."), *Body->GetName());
			continue;
		}
		NewBody->Rename(TEXT("UAGX_RigidBody"));
		NewBody->Mass = Body->GetMass();
		NewBody->MotionControl = Body->GetMotionControl();

		UAGX_BoxShapeComponent* NewBox = FAGX_EditorUtilities::CreateBoxShape(NewActor, Root);
		NewBox->HalfExtent = Box->GetHalfExtents(World);
	}

	for (auto& SphereBody : Archive.GetSphereBodies())
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX sphere body with name %s at %f."), *SphereBody.Body->GetName(),
			SphereBody.Body->GetPosition(World).X);

		const FRigidBodyBarrier* Body = SphereBody.Body;
		const FSphereShapeBarrier* Sphere = SphereBody.Sphere;

		/// \todo Consider using the state synchronization functions we already
		/// have, the ones used between time steps.

		FTransform Transform(Body->GetRotation(), Body->GetPosition(World));
		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Transform);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s'."), *Body->GetName());
			continue;
		}
		NewActor->SetActorLabel(Body->GetName());

		/// \todo I don't know what RF_Transactional means. Taken from UActorFactoryEmptyActor.
		/// Related to undo/redo, I think.
		USceneComponent* Root = NewObject<USceneComponent>(NewActor, RootName /*, RF_Transactional*/);
		NewActor->SetRootComponent(Root);
		NewActor->AddInstanceComponent(Root);
		Root->RegisterComponent();

		/// \todo For some reason the actor location must be set again after
		/// creating the root SceneComponent, or else the Actor remain at the
		/// origin. I'm assuming we must set rotation as well, but haven't
		/// tested yet.
		NewActor->SetActorLocation(Body->GetPosition(World));

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		NewBody->Rename(TEXT("AGX_RigidBody"));
		NewBody->Mass = Body->GetMass();
		NewBody->MotionControl = Body->GetMotionControl();

		UAGX_SphereShapeComponent* NewSphere = FAGX_EditorUtilities::CreateSphereShape(NewActor, Root);
		NewSphere->Radius = Sphere->GetRadius(World);
	}

	return ImportRoot;
}

#undef LOCTEXT_NAMESPACE
