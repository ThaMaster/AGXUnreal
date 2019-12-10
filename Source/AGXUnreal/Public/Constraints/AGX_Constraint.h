// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_ConstraintPropertyPerDof.h"
#include "Constraints/ConstraintBarrier.h" // TODO: Shouldn't be necessary here!

#include "AGX_Constraint.generated.h"

class FConstraintBarrier;

/**
 * Abstract base class for all AGX constraint types.
 *
 * Does not have a its own world space transform, but has references to the two
 * Rigid Body Actors to constrain to each other, and their attachment frames
 * (which defines how the rigid bodies should locally be jointed to one another).
 *
 * At least the first Rigid Body Actor must be chosen. If there is no second Rigid Body Actor,
 * then the first one will be constrained to the static World instead.
 *
 * Each non-abstract subclass must:
 *   1. Implement CreateNativeImpl (see method comment).
 *   2. In the constructor pass the constraint type specific array of locked DOFs to the
 *       overloaded AAGX_Constraint constructor. The array items and their indexes must exactly
 *       match the enum in the header of the native AGX constraint (without ALL_DOF and NUM_DOF).
 *
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Abstract, NotBlueprintable, meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_Constraint : public AActor
{
	GENERATED_BODY()

public:
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

private:
	UPROPERTY()
	class UAGX_ConstraintComponent* ConstraintComponent;

	UPROPERTY(Transient)
	class UAGX_ConstraintDofGraphicsComponent* DofGraphicsComponent;

	UPROPERTY(Transient)
	class UAGX_ConstraintIconGraphicsComponent* IconGraphicsComponent;

public:
	AAGX_Constraint()
	{
	}

	AAGX_Constraint(const TArray<EDofFlag>& LockedDofsOrdered);

	virtual ~AAGX_Constraint();

	UAGX_ConstraintDofGraphicsComponent* GetDofGraphics() const
	{
		return DofGraphicsComponent;
	}

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override
	{
		return false;
	}

	/**
	 * Returns true if for any of the locked DOFs both the global attachment frame transforms do no match.
	 *
	 * This function should never be used after the constraint has begun play.*
	 *
	 * Can be overriden for specialized constraint checks.
	 */
	virtual bool AreFramesInViolatedState(float Tolerance = KINDA_SMALL_NUMBER) const;

	EDofFlag GetLockedDofsBitmask() const;

	bool IsDofLocked(EDofFlag Dof) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Get the native AGX Dynamics representation of this constraint. Create it if necessary. */
	FConstraintBarrier* GetOrCreateNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	FConstraintBarrier* GetNative();

	/** Return true if the AGX Dynamics object has been created. False otherwise. */
	bool HasNative() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

protected:
#if WITH_EDITOR
	virtual void PostLoad() override; // When loaded in Editor/Game
	virtual void PostDuplicate(bool bDuplicateForPIE) override; // When copied in Editor
	virtual void OnConstruction(
		const FTransform& Transform) override; // When Loaded or Spawned in Editor, or Spawned in Game
	virtual void BeginDestroy() override; // When destroyed in Game
	virtual void Destroyed() override; // When destroyed in Editor
#endif

	virtual void BeginPlay() override;

	bool ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof);

	/** Must be overriden by derived class, and create a NativeBarrier with allocated native. Nothing more.*/
	virtual void CreateNativeImpl() PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl, );

	TUniquePtr<FConstraintBarrier> NativeBarrier;

	const EDofFlag LockedDofsBitmask = static_cast<EDofFlag>(0);

	// The Degrees of Freedom (DOF) that are locked by the specific constraint type,
	// ordered the way they are indexed by in the native AGX api (except for ALL_DOF and NUM_DOF).
	const TArray<EDofFlag> LockedDofs;

	// Mapping from EGenericDofIndex to native AGX constraint specific DOF index.
	// This list can change with each constraint type, and should exactly reflect
	// the DOF enum in the native header for each constraint.
	const TMap<EGenericDofIndex, int32> NativeDofIndexMap;

private:
	/** Invokes CreateNativeImpl, then adds the native to the simulation. */
	void CreateNative();
};
