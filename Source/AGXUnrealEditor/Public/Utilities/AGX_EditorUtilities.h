#pragma once

#include "CoreMinimal.h"

#include <tuple>

class AAGX_Constraint;
class AAGX_ConstraintFrameActor;
class UAGX_RigidBodyComponent;
class UAGX_SphereShapeComponent;
class UAGX_BoxShapeComponent;
class UAGX_TrimeshShapeComponent;

class AActor;
class FText;
class UClass;
class USceneComponent;
class UStaticMeshComponent;
class UWorld;

class FAGX_EditorUtilities
{
public:
	/**
	 * Create a new Actor with an empty USceneComponent as its RootComponent.
	 */
	static std::tuple<AActor*, USceneComponent*> CreateEmptyActor(const FTransform& Transform, UWorld* World);

	/**
	 * Create a new AGX Rigid Body Component as a child of the given Actor.
	 */
	static UAGX_RigidBodyComponent* CreateRigidBody(AActor* Owner);

	/**
	 * Create a new AGX Sphere Shape as child of the given Actor.
	 * The shape will be attached to the given USceneComponent.
	 */
	static UAGX_SphereShapeComponent* CreateSphereShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Box Shape as child of the given Actor.
	 * The shape will be attached to the given USceneComponent.
	 */
	static UAGX_BoxShapeComponent* CreateBoxShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new AGX Trimesh Shape as a child of the given actor.
	 * The shape will be attached to the given USceneComponent.
	 * A StaticMeshComponent is neither selected nor created.
	 * @see CreateStaticMesh.
	 */
	static UAGX_TrimeshShapeComponent* CreateTrimeshShape(AActor* Owner, USceneComponent* Outer);

	/**
	 * Create a new UStaticMeshComponent, along with the underlying StaticMesh
	 * asset, from the given mesh data. The UStaticMeshComponent will be added
	 * as a child to the given UAGX_TrimeshShapeComponent.
	 */
	static UStaticMeshComponent* CreateStaticMesh(
		UAGX_TrimeshShapeComponent* Outer, const TArray<FVector>& VertexPositions);

	/**
	 * Create a new constraint of the specified type.
	 */
	static AAGX_Constraint* CreateConstraint(UClass* ConstraintType, AActor* RigidBody1, AActor* RigidBody2,
		bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable);

	/**
	 * Create a new AGX Constraint Frame Actor. Set as child to specified Rigid Body, if available.
	 */
	static AAGX_ConstraintFrameActor* CreateConstraintFrameActor(
		AActor* ParentRigidBody, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable);

	/**
	 * Change current selection to the specific actor.
	 */
	static void SelectActor(AActor* Actor, bool DeselectPrevious = true);

	/**
	 * Display a temporary notification popup in the lower right corner of the Editor.
	 */
	static void ShowNotification(const FText& Text);

	/**
	 * Display a dialog box with an OK button.
	 */
	static void ShowDialogBox(const FText& Text);

	/**
	 * Return the Editor world, i.e. not the one that is potentially currently playing.
	 */
	static UWorld* GetEditorWorld();

	/**
	 * Return the currently playing world, or null if game is not playing.
	 *
	 * @remarks Playing world can be access directly from actors, using the GetWorld()
	 * function which returns the world they belong in. Therefore this function should
	 * typically only be used by Editor code that does not have direct access to an Actor.
	 */
	static UWorld* GetPlayingWorld();

	/**
	 * Return GetPlayingWorld if game is playing, else returns GetEditorWorld.
	 *
	 * @remarks Current world can be access directly from actors, using the GetWorld()
	 * function which returns the world they belong in. Therefore this function should
	 * typically only be used by Editor code that does not have direct access to an Actor.
	 */
	static UWorld* GetCurrentWorld();

	/**
	 * Find the first two found actors that has a UAGX_RigidBodyComponent in the current selection.
	 */
	static void GetRigidBodyActorsFromSelection(AActor** OutActor1, AActor** OutActor2);
};
