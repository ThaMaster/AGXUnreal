// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AGX_Simulation.generated.h"

/**
 * Manages an AGX simulation instance.
 * 
 * When an instance of this class exists, native AGX objects relating to the simulation
 * can be created (RigidBody, Constraint, ContactMaterial, etc). When this instance
 * is destroyed, all those native objects must be destroyed.
 *
 * Lifetime is bound to a GameInstance. Therefore, each playing GameInstance will have
 * exactly one UAGX_Simulation instance.
 * 
 * When not playing, the CDO (class default object) can be modified through the Editor UI.
 * This is useful for setting simulation properties like time step and contact materials.
 * When a GameInstance is started, the properties of the CDO will automatically be copied
 * over by Unreal Engine to the GameInstance specific UAGX_Simulation instance.
 *
 */
UCLASS()
class AGXUNREAL_API UAGX_Simulation : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: // OVERRIDES

	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
};
