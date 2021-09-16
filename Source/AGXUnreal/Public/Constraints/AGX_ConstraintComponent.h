#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_UpropertyDispatcher.h"
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
class AGXUNREAL_API UAGX_ConstraintComponent : public USceneComponent, public IAGX_NativeOwner
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
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies", Meta = (SkipUCSModifiedProperties))
	FAGX_ConstraintBodyAttachment BodyAttachment1;
	// SkipUCSModifiedProperties because we set OwningActor during creation but we still want to
	// allow the user to override that default from the Details Panel. Normally, Properties set
	// during creation become read-only.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	bool SetBody1(UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	void SetConstraintAttachmentLocation1(const FVector& BodyLocalLocation);

	/**
	 * The second Rigid Body bound by this constraint, and its Attachment Frame definition.
	 * If second Rigid Body is null, the first Rigid Body will be constrained to the World.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Bodies", Meta = (SkipUCSModifiedProperties))
	FAGX_ConstraintBodyAttachment BodyAttachment2;
	// SkipUCSModifiedProperties because we set OwningActor during creation but we still want to
	// allow the user to override that default from the Details Panel. Normally, Properties set
	// during creation become read-only.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	bool SetBody2(UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Attachment")
	void SetConstraintAttachmentLocation2(const FVector& BodyLocalLocation);

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	bool bEnable = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	void SetEnable(bool InEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	bool GetEnable() const;

	/**
	 * Solve type for this constraint. Valid is DIRECT (default for non-iterative solvers),
	 * ITERATIVE or DIRECT_AND_ITERATIVE where DIRECT_AND_ITERATIVE means that this constraint
	 * will be solved both direct and iterative.
	 *
	 * Note that solve type is ignored by iterative solvers.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	TEnumAsByte<enum EAGX_SolveType> SolveType;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	void SetSolveType(EAGX_SolveType InSolveType);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	EAGX_SolveType GetSolveType() const;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDoublePropertyPerDof Elasticity;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	void SetElasticity(EGenericDofIndex Index, float InElasticity);

	void SetElasticity(EGenericDofIndex Index, double InElasticity);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	float GetElasticityFloat(EGenericDofIndex Index) const;

	double GetElasticity(EGenericDofIndex Index) const;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintDoublePropertyPerDof Damping;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	void SetDamping(EGenericDofIndex Index, float InDamping);

	void SetDamping(EGenericDofIndex Index, double InDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	float GetDampingFloat(EGenericDofIndex Index) const;

	double GetDamping(EGenericDofIndex Index) const;

	UPROPERTY(EditAnywhere, Category = "AGX Constraint Dynamics")
	FAGX_ConstraintRangePropertyPerDof ForceRange;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	void SetForceRange(EGenericDofIndex Index, float RangeMin, float RangeMax);

	void SetForceRange(EGenericDofIndex Index, const FFloatInterval& InForceRange);

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	float GetForceRangeMin(EGenericDofIndex Index) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Dynamics")
	float GetForceRangeMax(EGenericDofIndex Index) const;

	FFloatInterval GetForceRange(EGenericDofIndex Index) const;

	void CopyFrom(const FConstraintBarrier& Barrier);

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

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	/** Get the native AGX Dynamics representation of this constraint. Create it if necessary. */
	FConstraintBarrier* GetOrCreateNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	FConstraintBarrier* GetNative();

	/** Get the native AGX Dynamics representation of this constraint. May return nullptr. */
	const FConstraintBarrier* GetNative() const;

	/** Subclasses that overrides this MUST invoke the parents version! */
	virtual void UpdateNativeProperties();

	void UpdateNativeElasticity();

	void UpdateNativeDamping();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	//~ Begin UObject interface.
	virtual void PostInitProperties() override;
	//~ End UObject interface.

	bool ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof) const;

#if WITH_EDITOR
	void InitPropertyDispatcher();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	//~ Begin UActorComponent Interface
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent Interface

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
	/**
	 * Pointer to the Barrier object that holds the FConstraintRef that holds the agx::ConstraintRef
	 * that holds the agx::Constraint.
	 *
	 * Most Components hold the Barrier by-value but here we use a pointer because we need virtual
	 * dispatch for AllocateNativeImpl, the member function that does the allocation of the AGX
	 * Dynamics constraint, i.e., agx::Hinge, etc. There may be other ways to achieve the same goal
	 * that doesn't require an extra indirection.
	 *
	 * NativeBarrier pointer is initialized in each specific Constraint Component subclass so a
	 * Barrier object is always available, just like for the Components that store the Barrier
	 * by-value, and just like them the Barrier may be empty, i.e., not had the AGX Dynamics object
	 * created yet.
	 */
	TUniquePtr<FConstraintBarrier> NativeBarrier;

#if WITH_EDITORONLY_DATA
	FAGX_UpropertyDispatcher<UAGX_ConstraintComponent> PropertyDispatcher;
#endif

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
