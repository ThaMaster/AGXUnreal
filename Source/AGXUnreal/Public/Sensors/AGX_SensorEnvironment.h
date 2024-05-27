// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/AGX_LidarSensorReference.h"
#include "Sensors/AGX_MeshEntityData.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_SensorEnvironment.generated.h"

class AAGX_Terrain;
class UAGX_SimpleMeshComponent;
class UInstancedStaticMeshComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup = "AGX", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_SensorEnvironment : public AActor
{
	GENERATED_BODY()

public:
	AAGX_SensorEnvironment();

	UPROPERTY(EditAnywhere, Category = "AGX Sensor Environment")
	TArray<FAGX_LidarSensorReference> LidarSensors;

	/*
	 * Objects in the Level that gets within the range of any added Lidar will automatically be
	 * added to this Environment if they can be detected.
	 * Objects will be detected if they have a Static Mesh Component using "Generate Overlap Events"
	 * and it has a Static Mesh with "Simple Collision" active.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (ExposeOnSpawn))
	bool bAutoAddObjects {true};

	/*
	 * If set to true, any Component (e.g. Static Mesh) that has Visibility invisible will be
	 * ignored when using Auto Add Objects. If set to false, the Component will be added regardless
	 * of visibility.
	 * If manually adding objects to the Sensor Environment using any of the Add... functions, this
	 * property will not have an affect, and the object will always be added.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (EditCondition = "bAutoAddObjects"))
	bool bIgnoreInvisibleObjects {true};

	/*
	 * Default LOD index used when reading Meshes that are added to this Environment.
	 * If set to a negative value, the highest valid LOD index is used (lowest resolution).
	 * If this LOD index does not exist for a Mesh that is added, the closest valid (and lower) LOD
	 * index is selected.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	int32 DefaultLODIndex {-1};

	/*
	 * Default Reflectivity set on objects that does not have a "AGXLidarReflectivity" parameter
	 * associated with it. Used for calculating Lidar intensity data.
	 *
	 * For Static Mesh Components and Instanced Static Mesh Components, if the
	 * Render Material in the first slot has a parameter "AGXLidarReflectivity", that value will be
	 * used, otherwise this DefaultReflectivity is used.
	 *
	 * For Terrains, the LidarReflectivity property of the Terrain Material will be
	 * used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	float DefaultReflectivity {0.6f};

	/*
	 * Whether or not the transform of added Instanced Static Meshes should be updated each Tick.
	 * Updating Instanced Static Mesh transforms comes with some perfomance cost, especially if
	 * a large number of instances are present.
	 * As an optimization, this can be disabled by setting this property to false. Note that any
	 * transformation change of an Instanced Static Mesh Instance during Play will not be reflected
	 * in the Lidar simulation.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment", AdvancedDisplay)
	bool UpdateAddedInstancedMeshesTransforms {true};

	/*
	 * Add a Static Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * (Optional) LOD determines the LOD index used when reading the given Mesh. If left to -1,
	 * the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMesh(UStaticMeshComponent* Mesh, int32 LOD = -1);

	/*
	 * Add an AGX Simple Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddAGXMesh(UAGX_SimpleMeshComponent* Mesh);

	/*
	 * Add all instances of an Instanced Static Mesh Component so that they can be detected by
	 * sensors handled by this Sensor Environment.
	 * (Optional) LOD determines the LOD index used when reading the given Mesh. If left to -1,
	 * the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMesh(UInstancedStaticMeshComponent* Mesh, int32 LOD = -1);

	/*
	 * Add a single instance of an Instanced Static Mesh Component so that it can be detected by
	 * sensors handled by this Sensor Environment. The Index corresponds to the Mesh Instance to
	 * add.
	 * (Optional) LOD determines the LOD index used when reading the given Mesh. If left to -1,
	 * the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index, int32 LOD = -1);

	/*
	 * Add a Terrain so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddTerrain(AAGX_Terrain* Terrain);

	/*
	 * Remove a Static Mesh Component from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveMesh(UStaticMeshComponent* Mesh);

	/*
	 * Remove an Instanced Static Mesh Component and all its instances from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh);

	/*
	 * Remove a single Instanced Static Mesh Instace from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index);

	/*
	 * Remove a Terrain from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveTerrain(AAGX_Terrain* Terrain);

	bool HasNative() const;
	FSensorEnvironmentBarrier* GetNative();
	const FSensorEnvironmentBarrier* GetNative() const;

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// ~End UObject interface.

	// ~Begin AActor interface.
	virtual void Tick(float DeltaSeconds) override;
	// ~End AActor interface.

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterLidars();
	void UpdateTrackedLidars();
	void UpdateTrackedMeshes();
	void UpdateTrackedInstancedMeshes();
	void UpdateTrackedAGXMeshes();
	void StepTrackedLidars() const;

	bool AddMesh(
		UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddMesh(
		UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddInstancedMesh(
		UInstancedStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddInstancedMeshInstance_Internal(UInstancedStaticMeshComponent* Mesh, int32 Index);

	UFUNCTION()
	void OnLidarBeginOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLidarEndOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void OnLidarBeginOverlapStaticMeshComponent(UStaticMeshComponent& Mesh);
	void OnLidarBeginOverlapInstancedStaticMeshComponent(
		UInstancedStaticMeshComponent& Mesh, int32 Index);
	void OnLidarBeginOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh);

	void OnLidarEndOverlapStaticMeshComponent(UStaticMeshComponent& Mesh);
	void OnLidarEndOverlapInstancedStaticMeshComponent(
		UInstancedStaticMeshComponent& Mesh, int32 Index);
	void OnLidarEndOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh);

private:
	// Todo: weak object ptr instead of raw ptrs!
	TMap<TWeakObjectPtr<UAGX_LidarSensorComponent>, TObjectPtr<USphereComponent>> TrackedLidars;
	TMap<TWeakObjectPtr<UStaticMeshComponent>, FAGX_MeshEntityData> TrackedMeshes;
	TMap<TWeakObjectPtr<UInstancedStaticMeshComponent>, FAGX_InstancedMeshEntityData>
		TrackedInstancedMeshes;
	TMap<TWeakObjectPtr<UAGX_SimpleMeshComponent>, FAGX_MeshEntityData> TrackedAGXMeshes;

	FSensorEnvironmentBarrier NativeBarrier;
	FDelegateHandle PostStepForwardHandle;
};
