#pragma once

#include "CoreMinimal.h"

class AActor;
class AAGX_Constraint;
class AAGX_ConstraintFrameActor;
class FText;
class UClass;
class UWorld;

static class FAGX_EditorUtilities
{
public:
	
	/**
	 * Create a new constraint of the specified type.
	 */
	static AAGX_Constraint* CreateConstraint(UClass* ConstraintType, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable);

	/**
	 * Create a new AGX Constraint Frame Actor. Set as child to specified Rigid  Body, if available.
	 */
	static AAGX_ConstraintFrameActor* CreateConstraintFrameActor(AActor* ParentRigidBody, bool bSelect, bool bShowNotification, bool bInPlayingWorldIfAvailable);

	/**
	 * Change current selection to the specific actor.
	 */
	static void SelectActor(AActor* Actor, bool DeselectPrevious = true);

	/**
	 * Display a temporary notification popup in the lower right corner of the Editor.
	*/
	static void ShowNotification(const FText &Text);

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