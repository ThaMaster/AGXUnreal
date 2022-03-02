// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_CollisionGroupPair.generated.h"

USTRUCT()
struct FAGX_CollisionGroupPair
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group1;

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group2;

	const bool operator==(const FAGX_CollisionGroupPair& Other) const
	{
		return Group1.IsEqual(Other.Group1) && Group2.IsEqual(Other.Group2);
	}

	bool IsEqual(const FName& GroupA, const FName& GroupB) const;
};
