// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackPropertiesBase.generated.h"

class UAGX_TrackPropertiesAsset;
class UAGX_TrackPropertiesInstance;

/**
 * Containing properties of an AGX Track Component.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable, abstract,
	AutoCollapseCategories = ("Hinge Compliance", "Hinge Damping"))
class AGXUNREAL_API UAGX_TrackPropertiesBase : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Compliance of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.000000000000000001", ClampMax = "1.0"))
	FAGX_Real HingeComplianceTranslational_X;

	/**
	 * Compliance of the hinges between track nodes, along the track direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.000000000000000001", ClampMax = "1.0"))
	FAGX_Real HingeComplianceTranslational_Y;

	/**
	 * Compliance of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis).
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.000000000000000001", ClampMax = "1.0"))
	FAGX_Real HingeComplianceTranslational_Z;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceTranslational_AsFloat(float X, float Y, float Z);
	virtual void SetHingeComplianceTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z);

	/**
	 * Compliance of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.000000000000000001", ClampMax = "1.0"))
	FAGX_Real HingeComplianceRotational_X;

	/**
	 * Compliance of the hinges between track nodes, around the track direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.000000000000000001", ClampMax = "1.0"))
	FAGX_Real HingeComplianceRotational_Y;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeComplianceRotational_AsFloat(float X, float Y);
	virtual void SetHingeComplianceRotational(FAGX_Real X, FAGX_Real Y);

	/**
	 * Damping of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeDampingTranslational_X;

	/**
	 * Damping of the hinges between track nodes, along the track direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeDampingTranslational_Y;

	/**
	 * Damping of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis).
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeDampingTranslational_Z;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeDampingTranslational_AsFloat(float X, float Y, float Z);
	virtual void SetHingeDampingTranslational(FAGX_Real X, FAGX_Real Y, FAGX_Real Z);

	/**
	 * Damping of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeDampingRotational_X;

	/**
	 * Damping of the hinges between track nodes, around the track direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeDampingRotational_Y;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeDampingRotational_AsFloat(float X, float Y);
	virtual void SetHingeDampingRotational(FAGX_Real X, FAGX_Real Y);

	/**
	 * Whether to enable the range in the hinges between the track nodes
	 * to define how far the track may bend.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range")
	bool bHingeRangeEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	virtual void SetHingeRangeEnabled(bool bEnable);

	/**
	 * Range used if the hinge range between the nodes is enabled. [degrees]
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range", Meta = (EditCondition = "bHingeRangeEnabled"))
	FAGX_RealInterval HingeRange;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeRangeMinMax(float Min, float Max);
	virtual void SetHingeRange(const FAGX_RealInterval& Range);

	/**
	 * When the track has been initialized some nodes are in contact with the wheels.
	 *
	 * If this flag is true the interacting nodes will be merged to the wheel directly after
	 * initialize, if false the nodes will be merged during the first (or later) time step.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bOnInitializeMergeNodesToWheelsEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	virtual void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable);

	/**
	 * Whether to position/transform the track nodes to the surface of the wheels after the
	 * track has been initialized. If false, the routing algorithm positions are used as is.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bOnInitializeTransformNodesToWheelsEnabled;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	virtual void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable);

	/**
	 * When the nodes are transformed to the wheels, this is the final target overlap. [cm]
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real TransformNodesToWheelsOverlap;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetTransformNodesToWheelsOverlap_AsFloat(float Overlap);
	virtual void SetTransformNodesToWheelsOverlap(FAGX_Real Overlap);

	/**
	 * Threshold when to merge a node to a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsMergeThreshold;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNodesToWheelsMergeThreshold_AsFloat(float MergeThreshold);
	virtual void SetNodesToWheelsMergeThreshold(FAGX_Real MergeThreshold);

	/**
	 * Threshold when to split a node from a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsSplitThreshold;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNodesToWheelsSplitThreshold_AsFloat(float SplitThreshold);
	virtual void SetNodesToWheelsSplitThreshold(FAGX_Real SplitThreshold);

	/**
	 * Average direction of non-merged nodes entering or exiting a wheel is used as
	 * reference direction to split of a merged node.
	 *
	 * This is the number of nodes to include into this average direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties", Meta = (ClampMin = "1"))
	uint32 NumNodesIncludedInAverageDirection;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	virtual void SetNumNodesIncludedInAverageDirection(int NumIncludedNodes);

	/**
	 * Minimum value of the normal force (the hinge force along the track) used in
	 * "internal" friction calculations.
	 *
	 * I.e., when the track is compressed, this value is used with the friction coefficient
	 * as a minimum stabilizing compliance. If this value is negative there will be
	 * stabilization when the track is compressed.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real MinStabilizingHingeNormalForce;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetMinStabilizingHingeNormalForce_AsFloat(float MinNormalForce);
	virtual void SetMinStabilizingHingeNormalForce(FAGX_Real MinNormalForce);

	/**
	 * Friction parameter of the internal friction in the node hinges.
	 *
	 * This parameter scales the normal force in the hinge.
	 *
	 * This parameter can not be identified as a real friction coefficient when it's used to
	 * stabilize tracks under tension.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real StabilizingHingeFrictionParameter;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetStabilizingHingeFrictionParameter_AsFloat(float FrictionParameter);
	virtual void SetStabilizingHingeFrictionParameter(FAGX_Real FrictionParameter);


public:
	/**
	 * Invokes the member function GetOrCreateInstance() on TrackProperties pointed to by Property,
	 * assigns the return value to Property, and then returns it. Returns null and does nothing if
	 * PlayingWorld is not an in-game world.
	 */
	static UAGX_TrackPropertiesInstance* GetOrCreateInstance(
		UWorld* PlayingWorld, UAGX_TrackPropertiesBase*& Property);

	UAGX_TrackPropertiesBase();

	virtual ~UAGX_TrackPropertiesBase();

	virtual UAGX_TrackPropertiesInstance* GetInstance()
		PURE_VIRTUAL(UAGX_TrackPropertiesBase::GetInstance, return nullptr;);

	/**
	 * If PlayingWorld is an in-game World and this TrackProperties is a UAGX_TrackPropertiesAsset,
	 * returns a UAGX_TrackPropertiesInstance representing the TrackProperties asset throughout the
	 * lifetime of the GameInstance. If this is already a UAGX_TrackPropertiesInstance it returns
	 * itself. Returns null if not in-game (invalid call).
	 */
	virtual UAGX_TrackPropertiesInstance* GetOrCreateInstance(UWorld * PlayingWorld)
		PURE_VIRTUAL(UAGX_TrackPropertiesBase::GetOrCreateInstance, return nullptr;);

	/**
	 * If this TrackProperties is a UAGX_TrackPropertiesInstance, returns the
	 * UAGX_TrackPropertiesAsset it was created from (if it still exists). Else returns null.
	 */
	virtual UAGX_TrackPropertiesAsset* GetAsset()
		PURE_VIRTUAL(UAGX_TrackPropertiesBase::GetAsset, return nullptr;);

	void CopyFrom(const UAGX_TrackPropertiesBase * Source);
	//void CopyFrom(const FTrackPropertiesBarrier * Source);

#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every Property in PostEditChangeProperty and PostEditChangeChainProperty.
	virtual void InitPropertyDispatcher();
#endif

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.
};
