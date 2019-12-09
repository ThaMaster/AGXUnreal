// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_ConstraintFrameActor.generated.h"

/**
 * Actor helper that can be used to define the location and rotation of
 * a Constraint Attachment Frame in a controlled way.
 *
 * Reference this object from an AGX Constraint.
 *
 * It can be referenced by both Rigid Bodies of the same AGX Constraint,
 * which makes it easy to position the constraint frame anywhere in the world.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_ConstraintFrameActor : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	AAGX_ConstraintFrameActor();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override
	{
		return false;
	}

private:
	UPROPERTY()
	class UAGX_ConstraintFrameComponent* ConstraintFrameComponent;

	/**
	 * The constraint(s) that are referencing this frame.
	 *
	 * Used for convenience only, to be able to quickly access the constraint(s).
	 */
	UPROPERTY(VisibleAnywhere, Transient)
	TArray<class AAGX_Constraint*> UsedByConstraints;

public:
	void AddConstraintUsage(AAGX_Constraint* Constraint);

	void RemoveConstraintUsage(AAGX_Constraint* Constraint);

	const TArray<class AAGX_Constraint*>& GetConstraintUsage() const;
};
