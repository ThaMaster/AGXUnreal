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

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (ExposeOnSpawn))
	bool bAutoAddObjects {true};	

	/*
	 * Add a Static Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMesh(UStaticMeshComponent* Mesh);

	/*
	 * Add an AGX Simple Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddAGXMesh(UAGX_SimpleMeshComponent* Mesh);

	/*
	 * Add all instances of an Instanced Static Mesh Component so that they can be detected by
	 * sensors handled by this Sensor Environment. Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMesh(UInstancedStaticMeshComponent* Mesh);

	/*
	 * Add a single instance of an Instanced Static Mesh Component so that it can be detected by
	 * sensors handled by this Sensor Environment. The Index corresponds to the Mesh Instance to
	 * add. Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index);

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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterLidars();
	void AutoStep();
	void Step(double DeltaTime);
	void StepNoAutoAddObjects(double DeltaTime);
	void StepAutoAddObjects(double DeltaTime);
	void UpdateTrackedLidars();
	void UpdateTrackedMeshes();
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
