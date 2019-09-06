// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGX_Constraint.generated.h"

class AAGX_ConstraintFrame;


/**
 * Abstract base class for all AGX constraint types.
 *
 * Does not have a its own world space transform, but has references to two
 * AAGX_ConstraintFrame that each has a transform. Will automatically find the
 * two AActors with a UAGX_RigidBodyComponent to apply the constraint to
 * by searching from each referenced AAGX_ConstraintFrame for the hierarchically
 * closest rigid body ancestor (or the world if no rigid body ancestor exists).
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Abstract,
	meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_Constraint : public AActor
{
	GENERATED_BODY()

public:

	/**
	 * The constraint attachment frame of the first Rigid Body bound by this constraint.
	 *
	 * The Rigid Body is the hierarchically closest Rigid Body ancestor of the frame,
	 * or the world if no Rigid Body ancestor exists.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	AAGX_ConstraintFrame *ConstraintFrame1;

	/**
	 * The constraint attachment frame of the second Rigid Body bound by this constraint.
	 *
	 * The Rigid Body is the hierarchically closest Rigid Body ancestor of the frame,
	 * or the world if no Rigid Body ancestor exists.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint")
	AAGX_ConstraintFrame *ConstraintFrame2;
	
public:	

	/** Sets default values for this actor's properties. */
	AAGX_Constraint();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override { return false; }

protected:

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

};
