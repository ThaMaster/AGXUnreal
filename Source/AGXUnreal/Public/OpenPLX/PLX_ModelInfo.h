// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLXModelInfo.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "PLX_ModelInfo.generated.h"


/**
 * Todo: add description.
 */
UCLASS(ClassGroup = "PLX", Category = "PLX")
class AGXUNREAL_API UPLX_ModelInfo : public ULevelInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UPLX_ModelInfo* GetFrom(UWorld* World);

	bool HasNative() const;
	FPLXModelInfo* GetNative();
	const FPLXModelInfo* GetNative() const;

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

	FPLXModelInfo Native;
};
