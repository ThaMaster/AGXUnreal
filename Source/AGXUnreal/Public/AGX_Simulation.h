// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_SimulationEnums.h"
#include "Contacts/ShapeContactBarrier.h"
#include "SimulationBarrier.h"

// Unreal Engine includes.
#include "Containers/Map.h"
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
	/**
	 * Step length of the integrator [s].
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Solver",
		Meta = (ClampMin = "0.001", UIMin = "0.001", ClampMax = "1.0", UIMax = "1.0"))
	float TimeStep = 1.0f / 60.0f;
	// BlueprintReadOnly because will need to make UFunctions for proper Set/GetTimeStep.

	/**
	 * Contact warmstarting adds an extra step to the solver where the previous timesteps contact
	 * data is matched to the current timesteps data so the previous forces can be used by the
	 * solver to find a solution faster when direct friction is used.
	 *
	 * If using auto-generated ContactMaterials and default settings, enabling this is not
	 * recommended and will just add some overhead. But if a FrictionModel with DIRECT solvetype is
	 * used, enabling this is recommended.
	 *
	 * The state data used to match contacts for warmstarting is not serialized. This means that
	 * storing and then restoring the simulation can give slightly different trajectories compared
	 * to running the simulation non-stop.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Solver")
	bool bContactWarmstarting = false;

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

	/** Specifies the gravity vector when using Uniform Gravity Field with magnitude [cm/s^2]. */
	UPROPERTY(
		Config, EditAnywhere, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Uniform"))
	FVector UniformGravity = FVector(0.0f, 0.0f, -980.665f);

	/** Specifies the world location towards which the gravity field is directed when using Point
	 * Gravity Field [cm]. */
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
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Simulation Stepping Mode")
	TEnumAsByte<enum EAGX_StepMode> StepMode = SmDropImmediately;

	/** Maximum time lag for the Catch up over time Capped step mode before dropping [s]. */
	UPROPERTY(Config, EditAnywhere, Category = "Simulation Stepping Mode")
	float TimeLagCap = 1.0;

	/** Set to true to enable statistics gathering in AGX Dynamics. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	bool bEnableStatistics = false;

	/**
	 * Globally enable or disable AMOR (Merge Split Handler) in AGX Dynamics.
	 * Note that each RigidBody / Geometry / Wire / Constraint need to enable merge/split
	 * individually for AMOR to be used for those.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AMOR")
	bool bEnableAMOR = true;

#if WITH_EDITORONLY_DATA
	/** TODO add comment */
	UPROPERTY(
		Config, EditAnywhere, Category = "AMOR",
		meta = (AllowedClasses = "AGX_ShapeContactMergeSplitThresholdsBase"))
	FSoftObjectPath GlobalShapeContactMergeSplitThresholds;
#endif


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

	/**
	 * Maximum distance between the active Viewport camera and any AGX Constraint within which
	 * the AGX Constraint graphical representation is scaled such that it's size is constant as
	 * drawn on the screen [cm].
	 *
	 * When moving the camera further away than this distance the on-screen size of the constraint
	 * visualization will start to shrink along with the surrounding geometry.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Rendering", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ConstraintVisualizationScalingDistanceMax = 400.f;

public: // Member functions.
	UFUNCTION(BlueprintCallable, Category = "Solver")
	void SetEnableContactWarmstarting(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Solver")
	bool GetEnableContactWarmstarting() const;

	UFUNCTION(BlueprintCallable, Category = "AMOR")
	void SetEnableAMOR(bool bEnable);

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

	/**
	 * Note that Shapes that are child of the passed Rigid Body are NOT added to the simulation
	 * when calling this function.
	 */
	void Add(UAGX_RigidBodyComponent& Body);
	void Add(UAGX_ShapeComponent& Shape);
	void Add(UAGX_ShapeMaterialInstance& Shape);
	void Add(UAGX_StaticMeshComponent& Body);
	void Add(AAGX_Terrain& Terrain);
	void Add(UAGX_TireComponent& Tire);
	void Add(UAGX_WireComponent& Wire);

	void Remove(UAGX_ConstraintComponent& Constraint);
	void Remove(UAGX_RigidBodyComponent& Body);
	void Remove(UAGX_ShapeComponent& Shape);
	void Remove(UAGX_ShapeMaterialInstance& Shape);
	void Remove(UAGX_StaticMeshComponent& Body);
	void Remove(AAGX_Terrain& Terrain);
	void Remove(UAGX_TireComponent& Tire);
	void Remove(UAGX_WireComponent& Wire);

	void Register(UAGX_ContactMaterialInstance& Material);
	void Unregister(UAGX_ContactMaterialInstance& Material);

	void SetEnableCollisionGroupPair(const FName& Group1, const FName& Group2, bool CanCollide);

	bool WriteAGXArchive(const FString& Filename) const;

	bool HasNative() const;

	FSimulationBarrier* GetNative();
	const FSimulationBarrier* GetNative() const;

	/**
	 * Perform a number of steps, possibly zero, in response to the elapsed Unreal Engine time
	 * according to the rules of the selected Step Mode.
	 *
	 * This member function is typically called automatically by an AAGX_Stepper instance. If you
	 * need precise control over stepping then set the Step Mode to None and call StepOnce to
	 * perform a step.
	 *
	 * @param DeltaTime The Unreal Engine time that has passed since the last frame.
	 */
	void Step(float DeltaTime);

	/**
	 * Step the AGX Dynamics simulation once. Typically used with the 'None' Step Mode to have full
	 * control over when the simulation is stepped. Does not do any delta time tracking so mixing
	 * automatic frame stepping, i.e. Step Mode != None, and StepOnce may step more than intended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void StepOnce();

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
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

private:
	int32 StepCatchUpImmediately(float DeltaTime);
	int32 StepCatchUpOverTime(float DeltaTime);
	int32 StepCatchUpOverTimeCapped(float DeltaTime);
	int32 StepDropImmediately(float DeltaTime);

	void EnsureStepperCreated();
	void EnsureValidLicense();

	void SetGravity();

#if WITH_EDITORONLY_DATA
	void SetGlobalNativeMergeSplitThresholds();
#endif

private:
	FSimulationBarrier NativeBarrier;

	/// Time that we couldn't step because DeltaTime was not an even multiple
	/// of the AGX Dynamics step size. That fraction of a time step is carried
	/// over to the next call to Step.
	float LeftoverTime;

	// The time it took to do a frame's stepping the last frame we actually took a step.
	double LastTotalStepTime {0.0};

	TWeakObjectPtr<AAGX_Stepper> Stepper;

	// Record for keeping track of the number of times any Contact Material has been
	// registered/unregistered. Value is incremented on Register() and decremented on Unregister().
	TMap<UAGX_ContactMaterialInstance*, int32> ContactMaterials;
};
