#pragma once

// AGXUnreal incldues.
#include "AGX_RigidBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"

#include "AGX_AgxEdModeConstraints.generated.h"

/**
 * Preset for setting which actor's transform should be used as attachment frames.
 */
UENUM()
enum class EAGX_ConstraintFrameSource
{
	/** Both Attachment Frames share the Constraint Actor's transform. */
	ConstraintTransform UMETA(DisplayName = "Constraint Actor Transform (recommended)"),

	/** Both Attachment Frames share the transform of an auto-created Constraint Frame Actor. */
	OneSharedFrameActor UMETA(DisplayName = "Shared Frame Actor"),

	/** Each Attachment Frame uses the transform of a uniquely created Constraint Frame Actor. */
	TwoFrameActors UMETA(DisplayName = "Individual Frame Actors"),

	/** Both Attachment Frames share the transform of the first Rigid Body Actor. */
	RigidBodyActor1 UMETA(DisplayName = "Rigid Body Actor 1"),

	/** Both Attachment Frames share the transform of the second Rigid Body Actor. */
	RigidBodyActor2 UMETA(DisplayName = "Rigid Body Actor 2"),

	/** Each Attachment Frame uses the transform of its Rigid Body Actor owner, with configurable
	   offsets. */
	LocalOnly UMETA(DisplayName = "Local Transforms Only")
};

/**
 * Preset for setting which actor the constraint should be attached to in the scene hierarchy.
 */
UENUM()
enum class EAGX_ConstraintActorParent
{
	/** Constraint Actor is attached to the first Rigid Body Actor. */
	RigidBodyActor1,

	/** Constraint Actor is attached to the first Rigid Body Actor. */
	RigidBodyActor2,

	/** Constraint Actor is not attached to any actor. */
	None,
};

/**
 * Sub-mode for AgxEdMode. Used to create and manage constraints.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeConstraints : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:
	static UAGX_AgxEdModeConstraints* GetInstance();

public:
	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;

public: // Constraint Creator
	UPROPERTY()
	UClass* ConstraintType;

	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	FAGX_RigidBodyReference RigidBody1;

	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	FAGX_RigidBodyReference RigidBody2;

	/**
	 * Which actor in the scene hierarchy should the Constraint Actor be attached to?
	 *
	 * Attaching the constraint to an actor can be convienient if the actor is moved around
	 * a lot while editing, and the constraint should follow.
	 *
	 * This can be changed afterwards.
	 */
	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	EAGX_ConstraintActorParent ConstraintParent;

	/**
	 * Which object's transform should be used as constraint attachment frame
	 * for each Rigid Body?
	 *
	 * Note that this is just an initial setting, for convenience.
	 * Attachment Frame Actors and local offsets can be edited in detail afterwards
	 * from the constraint's Details Window.
	 */
	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	EAGX_ConstraintFrameSource AttachmentFrameSource;

	/** Creates a new constraint using the current property values. */
	class AAGX_ConstraintActor* CreateConstraint() const;

public: // Constraint Browser
};
