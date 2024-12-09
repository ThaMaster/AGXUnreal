// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImporterSettings;

struct AGXUNREAL_API FAGX_ImportResult
{
	explicit FAGX_ImportResult(EAGX_ImportInstantiationResult InResult)
		: Result(InResult)
	{
	}

	FAGX_ImportResult(EAGX_ImportInstantiationResult InResult, AActor* InActor)
		: Result(InResult)
		, Actor(InActor)
	{
	}

	EAGX_ImportInstantiationResult Result {EAGX_ImportInstantiationResult::Invalid};
	AActor* Actor {nullptr};
};

class AGXUNREAL_API FAGX_Importer
{
public:
	FAGX_ImportResult Import(const FAGX_ImporterSettings& Settings);
};
