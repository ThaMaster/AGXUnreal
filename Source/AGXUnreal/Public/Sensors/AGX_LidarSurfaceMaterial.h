// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarMaterial.h"
#include "Sensors/RtSurfaceMaterialBarrier.h"

#include "AGX_LidarSurfaceMaterial.generated.h"

/**
 * Lidar Surface Material that can be assigned to Meshes in the scene. Determines the Lidar laser
 * ray interaction (e.g. intensity calculation) with such objects.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_LidarSurfaceMaterial : public UAGX_LidarMaterial
{
	GENERATED_BODY()

public:
	/**
	 * Reflectivity, between 0 and 1 where 0 means no laser light is reflected, and 1 means perfect
	 * reflection.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Reflectivity{0.7f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetReflectivity(float InReflectivity);

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	float GetReflectivity() const;

	bool HasNative() const;
	FRtSurfaceMaterialBarrier* GetNative();
	const FRtSurfaceMaterialBarrier* GetNative() const;

	void CommitToAsset();

	static UAGX_LidarSurfaceMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_LidarSurfaceMaterial& Source);

	UAGX_LidarSurfaceMaterial* GetOrCreateInstance(UWorld* PlayingWorld);
	FRtSurfaceMaterialBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	void UpdateNativeProperties();

	bool IsInstance() const;

	void CopyFrom(const FRtSurfaceMaterialBarrier& Source);
	void CopyProperties(const UAGX_LidarSurfaceMaterial& Source);

private:
	void CreateNative(UWorld* PlayingWorld);

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_LidarSurfaceMaterial> Asset;
	TWeakObjectPtr<UAGX_LidarSurfaceMaterial> Instance;
	FRtSurfaceMaterialBarrier NativeBarrier;
};
