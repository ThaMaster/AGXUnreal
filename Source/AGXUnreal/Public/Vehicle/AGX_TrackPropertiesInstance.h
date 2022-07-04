// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Vehicle/AGX_TrackPropertiesBase.h"

// AGX Dynamics for Unreal Barrier includes.
#include "Vehicle/TrackPropertiesBarrier.h" /// \todo Shouldn't be necessary here!

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackPropertiesInstance.generated.h"

class FTrackPropertiesBarrier;
class UAGX_TrackPropertiesAsset;

/**
 * Represents a native AGX TrackProperties in-game. Should never exist when not playing.
 *
 * Should only be created using the static function CreateFromAsset, which copyies data from its
 * sibling class UAGX_TrackPropertiesAsset.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Transient, NotPlaceable)
class AGXUNREAL_API UAGX_TrackPropertiesInstance : public UAGX_TrackPropertiesBase
{
	GENERATED_BODY()

public:
	static UAGX_TrackPropertiesInstance* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TrackPropertiesAsset* Source);

public:
	virtual ~UAGX_TrackPropertiesInstance();

	virtual UAGX_TrackPropertiesAsset* GetAsset() override;

	FTrackPropertiesBarrier* GetOrCreateNative(UWorld* PlayingWorld);

	FTrackPropertiesBarrier* GetNative();

	bool HasNative() const;

	void UpdateNativeProperties();

	// ~Begin UAGX_TrackPropertiesBase interface.
	virtual void SetHingeComplianceTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z) override;
	virtual void SetHingeComplianceRotational(FAGX_Real X, FAGX_Real Y) override;
	virtual void SetHingeDampingTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z) override;
	virtual void SetHingeDampingRotational(FAGX_Real X, FAGX_Real Y) override;
	virtual void SetHingeRangeEnabled(bool bEnable) override;
	virtual void SetHingeRange(const FAGX_RealInterval& Range) override;
	virtual void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable) override;
	virtual void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable) override;
	virtual void SetTransformNodesToWheelsOverlap(FAGX_Real Overlap) override;
	virtual void SetNodesToWheelsMergeThreshold(FAGX_Real MergeThreshold) override;
	virtual void SetNodesToWheelsSplitThreshold(FAGX_Real SplitThreshold) override;
	virtual void SetNumNodesIncludedInAverageDirection(int NumIncludedNodes)  override;
	virtual void SetMinStabilizingHingeNormalForce(FAGX_Real MinNormalForce) override;
	virtual void SetStabilizingHingeFrictionParameter(FAGX_Real InFrictionParameter) override;
	virtual UAGX_TrackPropertiesInstance* GetInstance() override;
	virtual UAGX_TrackPropertiesInstance* GetOrCreateInstance(UWorld* PlayingWorld) override;
	// ~End UAGX_TrackPropertiesBase interface.

private:
	// Creates the native AGX Contact Material. This function does not add the Native to the
	// Simulation.
	void CreateNative(UWorld* PlayingWorld);

	/// \todo This member is probably not necessary.. Remove it?
	TWeakObjectPtr<UAGX_TrackPropertiesAsset> SourceAsset;

	TUniquePtr<FTrackPropertiesBarrier> NativeBarrier;
};
