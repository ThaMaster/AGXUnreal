// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_ConstraintFrameActor.generated.h"


/**
 * Defines one of the two attachment frames and rigid bodies for a AAGX_Constraint.
 * The attachment frame is the transform of this actor, and the rigid body is the
 * hierarchically closest rigid body ancestor (or the world if no rigid body ancestor exists).
 */
UCLASS(ClassGroup = "AGX", Category = "AGX",
	meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_ConstraintFrameActor : public AActor
{
	GENERATED_BODY()

public:

	/** Sets default values for this actor's properties. */
	AAGX_ConstraintFrameActor();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override { return false; }

	UPROPERTY(VisibleDefaultsOnly, Meta = (Category = "AGX"))
	USceneComponent* Root;

private:

	/**
	 * The constraint(s) that are referencing this frame.
	 * 
	 * Used for convenience only, to be able to quickly access the constraint(s).
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<class AAGX_Constraint*> UsedByConstraints;

public:

	void AddConstraintUsage(AAGX_Constraint* Constraint);

	void RemoveConstraintUsage(AAGX_Constraint* Constraint);

};
