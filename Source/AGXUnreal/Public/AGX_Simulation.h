#pragma once

// AGX Dynamics for Unreal includes.
#include "SimulationBarrier.h"
#include "Contacts/ShapeContactBarrier.h"
#include "AGX_SimulationEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AGX_Simulation.generated.h"

class AAGX_Stepper;
class AAGX_Terrain;
class UAGX_ConstraintComponent;
class UAGX_ContactMaterialInstance;
class UAGX_MaterialBase;
class UAGX_RigidBodyComponent;
class UAGX_ShapeMaterialInstance;
class UAGX_StaticMeshComponent;
class UAGX_ShapeComponent;
class UAGX_TireComponent;
class UAGX_WireComponent;

class AActor;
class UActorComponent;
class UWorld;
class FShapeBarrier;

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

public: // Properties.
	/** Step length of the integrator, in seconds. 0.0167 by default. */
	UPROPERTY(
		config, EditAnywhere, BlueprintReadOnly, Category = "Solver",
		meta = (ClampMin = "0.001", UIMin = "0.001", ClampMax = "1.0", UIMax = "1.0"))
	float TimeStep = 1.0f / 60.0f;
	// BlueprintReadOnly because will need to make UFunctions for proper Set/GetTimeStep.

	/**
	 * Set to true to override the AGX Dynamics default value for the number of Parallel Projected
	 * Gauss-Seidel (PPGS) solver iterations. The number of iterations to use is set with Num PPGS
	 * Iterations.
	 */
	UPROPERTY(
		Config, EditAnywhere, Category = "Solver",
		Meta = (DisplayName = "Override PPGS Iterations"))
	bool bOverridePPGSIterations = false;

	/**
	 * The number of solver resting iterations to use for the Parallel Projected Gauss-Seidel (PPGS)
	 * solver. This value influences the accuracy and computation cost of e.g. terrain particles.
	 */
	UPROPERTY(
		Config, EditAnywhere, Category = "Solver",
		meta =
			(ClampMin = 1, UIMin = 1, DisplayName = "Num PPGS Iterations",
			 EditCondition = "bOverridePPGSIterations"))
	int32 NumPpgsIterations = 25;

	/** Specifies the gravity model used by the simulation. */
	UPROPERTY(Config, EditAnywhere, Category = "Gravity")
	TEnumAsByte<enum EAGX_GravityModel> GravityModel = EAGX_GravityModel::Uniform;

	/** Specifies the gravity vector when using Uniform Gravity Field with magnitude given in
	 * [cm/s^2]. */
	UPROPERTY(
		Config, EditAnywhere, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Uniform"))
	FVector UniformGravity = FVector(0.0f, 0.0f, -980.665f);

	/** Specifies the world location towards which the gravity field is directed when using Point
	 * Gravity Field. */
	UPROPERTY(
		Config, EditAnywhere, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Point"))
	FVector PointGravityOrigin = FVector::ZeroVector;

	/** Specifies the gravity magnitude when using Point Gravity Field [cm/s^2]. */
	UPROPERTY(
		Config, EditAnywhere, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Point"))
	float PointGravityMagnitude = -980.665f;

	/**
	 * Simulation stepping mode. This controls what happens when the simulation is unable to keep
	 * up with real-time.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Simulation Stepping Mode")
	TEnumAsByte<enum EAGX_StepMode> StepMode = SmCatchUpImmediately;

	/** Maximum time lag in seconds for the Catch up over time Capped step mode before dropping. */
	UPROPERTY(Config, EditAnywhere, Category = "Simulation Stepping Mode")
	float TimeLagCap = 1.0;

	/** Set to true to enable statistics gathering in AGX Dynamics. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	bool bEnableStatistics = false;

#if WITH_EDITORONLY_DATA
	/**
	 * Set to true to write an AGX Dynamics for Unreal archive of the initial state.
	 * The archive is written to the path set in ExportPath on the first game Tick.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Startup")
	bool bExportInitialState = false;

	/**
	 * Path to the file to which the initial AGX Dynamics state is written if ExportInitialState is
	 * set.
	 */
	UPROPERTY(
		Config, EditAnywhere, Category = "Startup", meta = (EditCondition = "bExportInitialState"))
	FString ExportPath;
#endif

	/**
	 * Remote debugging allows agxViewer, the default scene viewer in AGX
	 * Dynamics, to connect to the AGX_Simulation running inside Unreal Engine
	 * and render the internal simulation state using its built-in debug
	 * rendering capabilities.
	 *
	 * To connect to a running Unreal Engine instance launch agxViewer with
	 *    agxViewer -p --connect localhost:<PORT>
	 * where <PORT> is the port number configured in Project Settings > Plugins >  AGX Dynamics >
	 * Debug > RemoteDebuggingPort.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	uint8 bRemoteDebugging : 1;

	/** Network port to use for remote debugging. */
	UPROPERTY(Config, EditAnywhere, Category = "Debug", meta = (EditCondition = "bRemoteDebugging"))
	int16 RemoteDebuggingPort;

public: // Member functions.
	/**
	 * Set the number of solver resting iterations to use for the Parallel Projected Gauss-Seidel
	 * (PPGS) solver. This value influences the accuracy and computation cost of e.g. terrain
	 * particles.
	 * @param NumIterations The number of PPGS iterations to perform each solve.
	 */
	UFUNCTION(BlueprintCallable, Category = "Solver")
	void SetNumPpgsIterations(int32 NumIterations);

	/**
	 * \return The number of Parallel Projected Gauss-Seidel solver iterations to perform per
	 * solve.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Solver")
	int32 GetNumPpgsIterations();

	/**
	 * Get a collection of statistics from the AGX Dynamics simulation.
	 * These will likely be split into several functions in the future.
	 */
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	FAGX_Statistics GetStatistics();

	/**
	 * Returns the current time within the AGX Dynamics simulation world.
	 *
	 * The returned value is usually the amount of time that has been simulated, but calls to
	 * SetTimeStamp will invalidate this.
	 *
	 * The AGX Dynamics time tries to follow the Unreal Engine world time, using the DeltaTime
	 * values passed to the Tick callbacks, but some of the Step Modes allow for the two times to
	 * diverge.
	 *
	 * @return The current simulation time stamp.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	float GetTimeStamp() const;

	/**
	 * Set the current simulation time stamp. Does not simulate to that time, just moves the clock
	 * hands.
	 * @param NewTimeStamp The new time stamp.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void SetTimeStamp(float NewTimeStamp);

	void Add(UAGX_ConstraintComponent& Constraint);
	void Add(UAGX_ContactMaterialInstance& Material);
	void Add(UAGX_RigidBodyComponent& Body);

	/**
	 * Add a stand-alone shape to the simulation.
	 *
	 * Should not be called with Shapes that are part of a RigidBody, the body
	 * is responsible for adding its own shapes.
	 */
	void Add(UAGX_ShapeComponent& Shape);

	void Add(UAGX_ShapeMaterialInstance& Shape);
	void Add(UAGX_StaticMeshComponent& Body);
	void Add(AAGX_Terrain& Terrain);
	void Add(UAGX_TireComponent& Tire);
	void Add(UAGX_WireComponent& Wire);

	void Remove(UAGX_ConstraintComponent& Constraint);
	void Remove(UAGX_ContactMaterialInstance& Material);
	void Remove(UAGX_RigidBodyComponent& Body);

	/**
	 * Remove a stand-alone shape from the simulation.
	 *
	 * Should not be called with Shapes that are part of a RigidBody, the body
	 * is responsible for removing its own shapes.
	 */
	void Remove(UAGX_ShapeComponent& Shape);

	void Remove(UAGX_ShapeMaterialInstance& Shape);
	void Remove(UAGX_StaticMeshComponent& Body);
	void Remove(AAGX_Terrain& Terrain);
	void Remove(UAGX_TireComponent& Tire);
	void Remove(UAGX_WireComponent& Wire);

	void SetEnableCollisionGroupPair(const FName& Group1, const FName& Group2, bool CanCollide);

	bool WriteAGXArchive(const FString& Filename) const;

	bool HasNative() const;

	FSimulationBarrier* GetNative();
	const FSimulationBarrier* GetNative() const;

	void Step(float DeltaTime);

	static UAGX_Simulation* GetFrom(const UActorComponent* Component);

	static UAGX_Simulation* GetFrom(const AActor* Actor);

	static UAGX_Simulation* GetFrom(const UWorld* World);

	static UAGX_Simulation* GetFrom(const UGameInstance* GameInstance);

	TArray<FShapeContactBarrier> GetShapeContacts(const FShapeBarrier& Shape) const;

#if WITH_EDITOR
	virtual bool CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
		const UProperty* InProperty
#else
		const FProperty* InProperty
#endif
	) const override;
#endif

private:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;

private:
	void StepCatchUpImmediately(float DeltaTime);
	void StepCatchUpOverTime(float DeltaTime);
	void StepCatchUpOverTimeCapped(float DeltaTime);
	void StepDropImmediately(float DeltaTime);

	void EnsureStepperCreated();
	void EnsureLicenseChecked();

	void SetGravity();

private:
	FSimulationBarrier NativeBarrier;

	bool IsLicenseChecked = false;

	/// Time that we couldn't step because DeltaTime was not an even multiple
	/// of the AGX Dynamics step size. That fraction of a time step is carried
	/// over to the next call to Step.
	float LeftoverTime;

	TWeakObjectPtr<AAGX_Stepper> Stepper;
};
