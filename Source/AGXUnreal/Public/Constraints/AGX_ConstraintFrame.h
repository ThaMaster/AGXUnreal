#if 0
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_ConstraintFrame.generated.h"


/**
 * Defines one of the two attachment frames and rigid bodies for a AAGX_Constraint.
 * The attachment frame is the transform of this actor, and the rigid body is the
 * hierarchically closest rigid body ancestor (or the world if no rigid body ancestor exists).
 */
UCLASS(ClassGroup = "AGX", Category = "AGX",
	meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_ConstraintFrame : public AActor
{
	GENERATED_BODY()

public:

	/** Sets default values for this actor's properties. */
	AAGX_ConstraintFrame();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override { return false; }

	UPROPERTY(VisibleDefaultsOnly, Meta = (Category = "AGX"))
	USceneComponent* Root;

protected:

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

};
#endif