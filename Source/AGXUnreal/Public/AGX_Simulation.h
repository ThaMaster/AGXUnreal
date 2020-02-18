// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SimulationBarrier.h"

#include "AGX_Simulation.generated.h"

class UAGX_RigidBodyComponent;
class UAGX_MaterialBase;
class AAGX_Terrain;

/**
 * Manages an AGX simulation instance.
 *
 * When an instance of this class exists, native AGX objects relating to the
 * simulation can be created (RigidBody, Constraint, ContactMaterial, etc). When
 * this instance is destroyed, all those native objects must be destroyed.
 *
 * Lifetime is bound to a GameInstance. Therefore, each playing GameInstance
 * will have exactly one UAGX_Simulation instance.
 *
 * When not playing, the CDO (class default object) can be modified through the
 * Editor UI. From the toolbar select Settings -> Project Settings -> Plugins ->
 * AGX Dynamics. This is useful for setting simulation properties like time step
 * and contact materials. When a GameInstance is started, the properties of the
 * CDO will automatically be copied over by Unreal Engine to the GameInstance
 * specific UAGX_Simulation instance.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = Engine, defaultconfig)
class AGXUNREAL_API UAGX_Simulation : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Step length of the integrator, in seconds. 0.0167 by default. */
	UPROPERTY(
		config, EditAnywhere, Category = "Solver",
		meta = (ClampMin = "0.001", UIMin = "0.001", ClampMax = "1.0", UIMax = "1.0"))
	float TimeStep = 1.0f / 60.0f;

	/** Uniform default scene gravity, in cm/s^2. -980.665 by default. */
	UPROPERTY(config, EditAnywhere, Category = "Scene Defaults")
	FVector Gravity = FVector(0.0f, 0.0f, -980.665f);

	/**
	 * Remote debugging allows agxViewer, the default scene viewer in AGX
	 * Dynamics, to connect to the AGX_Simulation running inside Unreal Engine
	 * and render the internal simulation state using its built-in debug
	 * rendering capabilities.
	 *
	 * To connect to a running Unreal Engine instance launch agxViewer with
	 *    agxViewer -p --connect localhost:<PORT>
	 * where <PORT> is the port number configured in Project Settings -> Plugins
	 * -> AGX Dynamics -> Debug -> RemoteDebuggingPort.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	uint8 bRemoteDebugging : 1;

	/** Network port to use for remote debugging. */
	UPROPERTY(Config, EditAnywhere, Category = "Debug", meta = (EditCondition = "bRemoteDebugging"))
	int16 RemoteDebuggingPort;

	void AddRigidBody(UAGX_RigidBodyComponent* Body);
	void AddTerrain(AAGX_Terrain* Terrain);

	void SetDisableCollisionGroupPair(const FName& Group1, const FName& Group2);

	bool WriteAGXArchive(const FString& Filename) const;

	bool HasNative() const;

	FSimulationBarrier* GetNative();

	const FSimulationBarrier* GetNative() const;

	void Step(float DeltaTime);

	static UAGX_Simulation* GetFrom(const AActor* Actor);

	static UAGX_Simulation* GetFrom(const UWorld* World);

	static UAGX_Simulation* GetFrom(const UGameInstance* GameInstance);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;

private:
	FSimulationBarrier NativeBarrier;

	/// Time that we couldn't step because DeltaTime was not an even multiple
	/// of the AGX Dynamics step size. That fraction of a time step is carried
	/// over to the next call to Step.
	float LeftoverTime;
};
