#pragma once

// AGXUnreal includes.
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_ConstraintPropertyPerDof.h"
#include "Constraints/ConstraintBarrier.h" // TODO: Shouldn't be necessary here!

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_ConstraintComponent.generated.h"

class FConstraintBarrier;
class UAGX_ConstraintDofGraphicsComponent;
class UAGX_ConstraintIconGraphicsComponent;

/**
 * Component owned by every Constraint Actor so that component features can be used.
 * For example, enables the usage of a Component Visualizer, so that helpful graphics
 * can be shown in the Level Editor Viewport when editing the constraint.
 *
 * @see FAGX_ConstraintComponentVisualizer
 *
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ConstraintComponent();

protected:
	UAGX_ConstraintComponent(const TArray<EDofFlag>& LockedDofsOrdered);

public:
	virtual ~UAGX_ConstraintComponent();

	/**
	 * The first Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * Rigid Body Actor must be set.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies")
	FAGX_ConstraintBodyAttachment BodyAttachment1;

	/**
	 * The second Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * If second Rigid Body is null, the first Rigid Body will be constrained to the World.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies")
	FAGX_ConstraintBodyAttachment BodyAttachment2;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	bool bEnable;

	/**
	 * Solve type for this constraint. Valid is DIRECT (default for non-iterative solvers),
	 * ITERATIVE or DIRECT_AND_ITERATIVE where DIRECT_AND_ITERATIVE means that this constraint
	 * will be solved both direct and iterative.
	 *
	 * Note that solve type is ignored by iterative solvers.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	TEnumAsByte<enum EAGX_SolveType> SolveType;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDoublePropertyPerDof Elasticity;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDoublePropertyPerDof Damping;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintRangePropertyPerDof ForceRange;

	UAGX_ConstraintDofGraphicsComponent* GetDofGraphics1() const
	{
		return DofGraphicsComponent1;
	}

	UAGX_ConstraintDofGraphicsComponent* GetDofGraphics2() const
	{
		return DofGraphicsComponent2;
	}

	/**
	 * Returns true if for any of the locked DOFs both the global attachment frame transforms do no
	 * match.
	 *
	 * This function should never be used after the constraint has begun play.*
	 *
	 * Can be overriden for specialized constraint checks.
	 */
	virtual bool AreFramesInViolatedState(
		float Tolerance = KINDA_SMALL_NUMBER, FString* OutMessage = nullptr) const;

	EDofFlag GetLockedDofsBitmask() const;

	bool IsDofLocked(EDofFlag Dof) const;

	/** Get the native AGX Dynamics representation of this constraint. Create it if necessary. */
	FConstraintBarrier* GetOrCreateNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	FConstraintBarrier* GetNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	const FConstraintBarrier* GetNative() const;

	/** Return true if the AGX Dynamics object has been created. False otherwise. */
	bool HasNative() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/// \todo Determine which of these are needed, which are even available on USceneCompnent, and
	/// which must be hidden behind WITH_EDITOR.
protected:
#if WITH_EDITOR
	// When loaded in Editor/Game.
	virtual void PostLoad() override;

	// When copied in Editor.
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	// When destroyed in Game.
	virtual void BeginDestroy() override;

	// When destroyed in Editor.
	virtual void DestroyComponent(bool bPromoteChildren) override;

/// \note The Actor version of constraints had this following callbacks.
/// Components don't have it. Does it matter? Do we need it?
#if 0
	// When Loaded or Spawned in Editor, or Spawned in Game
	virtual void OnConstruction(const FTransform& Transform) override;

	// When destroyed in Editor.
	virtual void Destroyed() override;
#endif
#endif

protected:
	bool ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof);

	/**
	 * Allocate a type-specific native constraint and point NativeBarrier to it. Perform any
	 * constraint-specific configuration that may be necessary, such as binding secondary constraint
	 * barriers to their respective native objects within the native constraint.
	 */
	virtual void CreateNativeImpl() PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl, );

private:
	/**
	 * Invokes CreateNativeImpl, then adds the native to the simulation.
	 */
	void CreateNative();

protected:
	TUniquePtr<FConstraintBarrier> NativeBarrier;

private:
	const EDofFlag LockedDofsBitmask = static_cast<EDofFlag>(0);

	// The Degrees of Freedom (DOF) that are locked by the specific constraint type,
	// ordered the way they are indexed by in the native AGX api (except for ALL_DOF and NUM_DOF).
	const TArray<EDofFlag> LockedDofs;

	// Mapping from EGenericDofIndex to native AGX constraint specific DOF index.
	// This list can change with each constraint type, and should exactly reflect
	// the DOF enum in the native header for each constraint.
	const TMap<EGenericDofIndex, int32> NativeDofIndexMap;

	// It may not be possible to have these as sub-components. In that case we
	// must move all functionality from them into this class.
	UPROPERTY()
	UAGX_ConstraintIconGraphicsComponent* IconGraphicsComponent;

	UPROPERTY()
	UAGX_ConstraintDofGraphicsComponent* DofGraphicsComponent1;

	UPROPERTY()
	UAGX_ConstraintDofGraphicsComponent* DofGraphicsComponent2;
};
