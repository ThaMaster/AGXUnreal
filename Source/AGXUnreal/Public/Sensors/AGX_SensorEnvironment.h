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
	bool bAutoStep {true};

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (ExposeOnSpawn))
	bool bAutoAddObjects {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void Step(double DeltaTime);

	/*
	 * Add a Static Mesh Component so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMesh(UStaticMeshComponent* Mesh);

	/*
	 * Add a Terrain so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddTerrain(AAGX_Terrain* Terrain);

	bool Add(
		UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	/*
	 * Remove a Static Mesh Component from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool Remove(UStaticMeshComponent* Mesh);

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
	void StepNoAutoAddObjects(double DeltaTime);
	void StepAutoAddObjects(double DeltaTime);
	void UpdateTrackedLidars();
	void UpdateTrackedStaticMeshes();
	void StepTrackedLidars() const;

	UFUNCTION()
	void OnLidarBeginOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLidarEndOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:

	// Todo: weak object ptr instead of raw ptrs?
	TMap<UAGX_LidarSensorComponent*, USphereComponent*> TrackedLidars;
	TMap<UStaticMeshComponent*, FAGX_MeshEntityData> TrackedStaticMeshes;

	FSensorEnvironmentBarrier NativeBarrier;
	FDelegateHandle PostStepForwardHandle;
};
