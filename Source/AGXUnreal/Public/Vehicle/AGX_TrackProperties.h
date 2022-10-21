// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "UObject/Object.h"

#include "AGX_TrackProperties.generated.h"

/**
 * Contains configuration properties of an AGX Track Component. It is an asset that is created from
 * the Content Browser. Several Tracks can share the same Track Properties asset. Default values for
 * all properties has been taken from AGX Dynamics.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType,
	AutoCollapseCategories = ("Hinge Compliance", "Hinge Damping"))
class AGXUNREAL_API UAGX_TrackProperties : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Compliance of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node [m/N].
	 */
	UPROPERTY(
		EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_X = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, along the track direction [m/N].
	 */
	UPROPERTY(
		EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_Y = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis) [m/N].
	 */
	UPROPERTY(
		EditAnywhere, Category = "Hinge Compliance",
		Meta =
			(DisplayName = "SetHingeComplianceTranslational", ClampMin = "0.0"))
	FAGX_Real HingeComplianceTranslational_Z = DefaultHingeCompliance;

	void SetHingeComplianceTranslational(
		double ComplianceX, double ComplianceY, double ComplianceZ);
	void SetHingeComplianceTranslationalX(double ComplianceX);
	void SetHingeComplianceTranslationalY(double ComplianceY);
	void SetHingeComplianceTranslationalZ(double ComplianceZ);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Hinge Compliance Translational"))
	void SetHingeComplianceTranslational_BP(float X, float Y, float Z);

/// @todo Getter functions still work-in-progress.
#if 0
	double GetHingeComplianceTranslationalX() const;
	double GetHingeComplianceTranslationalY() const;
	double GetHingeComplianceTranslationalZ() const;
#endif

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (Displayname = "Get Hinge Compliance Translational"))
	void GetHingeComplianceTranslational_BP(float& X, float& Y, float& Z);

	/**
	 * Compliance of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node [rad/Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceRotational_X = DefaultHingeCompliance;

	/**
	 * Compliance of the hinges between track nodes, around the track direction [rad/Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "Hinge Compliance",
		Meta = (ClampMin = "0.0"))
	FAGX_Real HingeComplianceRotational_Y = DefaultHingeCompliance;

	void SetHingeComplianceRotational(double ComplianceX, double ComplianceY);
	void SetHingeComplianceRotationalX(double Compliance);
	void SetHingeComplianceRotationalY(double Compliance);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Hinge Compliance Rotational"))
	void SetHingeComplianceRotational_BP(float X, float Y);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Get Hinge Compliance Rotational"))
	void GetHingeComplianceRotational_BP(float& X, float& Y);

	/**
	 * Spook damping of the hinges between track nodes, along the axis pointing vertically
	 * out from the track node [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_X = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, along the track direction [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_Y = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, along the axis pointing sideways
	 * (i.e. the rotation axis) [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingTranslational_Z = DefaultHingeSpookDamping;

	void SetHingeSpookDampingTranslational(double DampingX, double DampingY, double DampingZ);
	void SetHingeSpookDampingTranslationalX(double Damping);
	void SetHingeSpookDampingTranslationalY(double Damping);
	void SetHingeSpookDampingTranslationalZ(double Damping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Hinge Spook Damping Translational"))
	void SetHingeSpookDampingTranslational_BP(float DampingX, float DampingY, float DampingZ);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Get Hinge Spook Damping Translational"))
	void GetHingeSpookDampingTranslational_BP(float& DampingX, float& DampingY, float& DampingZ);

	/**
	 * Spook damping of the hinges between track nodes, around the axis pointing vertically
	 * out from the track node [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingRotational_X = DefaultHingeSpookDamping;

	/**
	 * Spook damping of the hinges between track nodes, around the track direction [s].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Damping", Meta = (ClampMin = "0.0"))
	FAGX_Real HingeSpookDampingRotational_Y = DefaultHingeSpookDamping;

	void SetHingeSpookDampingRotational(double DampingX, double DampingY);
	void SetHingeSpookDampingRotationalX(double Damping);
	void SetHingeSpookDampingRotationalY(double Damping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Hinge Spook Damping Rotational"))
	void SetHingeSpookDampingRotational_BP(float X, float Y);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Get Hinge Spook Damping Rotational"))
	void GetHingeSpookDampingRotational_BP(float& X, float& Y);

	/**
	 * Whether to enable the range in the hinges between the track nodes
	 * to define how far the track may bend.
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range")
	bool bEnableHingeRange = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetHingeRangeEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	bool GetHingeRangeEnabled() const;

	/**
	 * Range used if the hinge range between the nodes is enabled [degrees].
	 */
	UPROPERTY(EditAnywhere, Category = "Hinge Range", Meta = (EditCondition = "bEnableHingeRange"))
	FAGX_RealInterval HingeRange {-120, 20};

	void SetHingeRangeRange(FAGX_RealInterval InHingeRange);

	void SetHingeRangeRange(double Min, double Max);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Hinge Range"))
	void SetHingeRangeRange_BP(float Min, float Max);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Get Hinge Range"))
	void GetHingeRangeRange_BP(float& Min, float& Max) const;

	/**
	 * When the track has been initialized some nodes are in contact with the wheels.
	 *
	 * If this flag is true the interacting nodes will be merged to the wheel directly after
	 * initialize, if false the nodes will be merged during the first (or later) time step.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bEnableOnInitializeMergeNodesToWheels = false;

	UFUNCTION(BlueprintCallable, Category = "Merge/Split Properties")
	void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Merge/Split Properties")
	bool GetOnInitializeMergeNodesToWheelsEnabled() const;

	/**
	 * Whether to position/transform the track nodes to the surface of the wheels after the
	 * track has been initialized. If false, the routing algorithm positions are used as is.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	bool bEnableOnInitializeTransformNodesToWheels = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	bool GetOnInitializeTransformNodesToWheelsEnabled() const;

	/**
	 * When the nodes are transformed to the wheels, this is the final target overlap [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real TransformNodesToWheelsOverlap = 0.1;

	void SetTransformNodesToWheelsOverlap(double Overlap);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Transform Nodes To Wheels Overlap"))
	void SetTransformNodesToWheelsOverlap_BP(float Overlap);

	double GetTransformNodesToWheelsOverlap() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Get Transform Nodes To Wheels Overlap"))
	float GetTransformNodesToWheelsOverlap_BP() const;

	/**
	 * Threshold when to merge a node to a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsMergeThreshold = -0.1;

	void SetNodesToWheelsMergeThreshold(double MergeThreshold);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Nodes To Wheels Merge Threshold"))
	void SetNodesToWheelsMergeThreshold_BP(float MergeThreshold);

	/**
	 * Threshold when to split a node from a wheel.
	 *
	 * Given a reference direction in the track, this value is the projection of the deviation
	 * (from the reference direction) of the node direction onto the wheel radial direction vector.
	 *
	 * I.e., when the projection is negative the node can be considered "wrapped" on the wheel.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties")
	FAGX_Real NodesToWheelsSplitThreshold = -0.05;

	void SetNodesToWheelsSplitThreshold(double SplitThreshold);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Nodes To Wheels Split Threshold"))
	void SetNodesToWheelsSplitThreshold_BP(float SplitThreshold);

	/**
	 * Average direction of non-merged nodes entering or exiting a wheel is used as
	 * reference direction to split of a merged node.
	 *
	 * This is the number of nodes to include into this average direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Merge/Split Properties", Meta = (ClampMin = "1"))
	uint32 NumNodesIncludedInAverageDirection = 3;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	void SetNumNodesIncludedInAverageDirection(int32 NumIncludedNodes);

	UFUNCTION(BlueprintCallable, Category = "AGX Track Properties")
	int32 GetNumNodesIncludedInAverageDirection() const;

	/**
	 * Minimum value of the normal force (the hinge force along the track) used in
	 * "internal" friction calculations [N].
	 *
	 * I.e., when the track is compressed, this value is used with the friction coefficient
	 * as a minimum stabilizing compliance. If this value is negative there will be
	 * stabilization when the track is compressed.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real MinStabilizingHingeNormalForce = 100.0;

	void SetMinStabilizingHingeNormalForce(double MinNormalForce);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DislayName = "Set Min Stabilizing Hinge Normal Force"))
	void SetMinStabilizingHingeNormalForce_BP(float MinNormalForce);

	/**
	 * Friction parameter of the internal friction in the node hinges.
	 *
	 * This parameter scales the normal force in the hinge.
	 *
	 * This parameter can not be identified as a real friction coefficient when it's used to
	 * stabilize tracks under tension.
	 */
	UPROPERTY(EditAnywhere, Category = "Stabilizing Properties", Meta = (ClampMin = "0.0"))
	FAGX_Real StabilizingHingeFrictionParameter = 1.0;

	void SetStabilizingHingeFrictionParameter(double FrictionParameter);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Track Properties",
		Meta = (DisplayName = "Set Stabilizing Hinge Friction Parameter"))
	void SetStabilizingHingeFrictionParameter_BP(float FrictionParameter);

public:
	UAGX_TrackProperties() = default;
	virtual ~UAGX_TrackProperties() = default;

	void CommitToAsset();

	void CopyFrom(const UAGX_TrackProperties* Source);
	void CopyFrom(const FTrackPropertiesBarrier& Source);

	/**
	 * Create the Play instance for the given Source Track Properties, which should be an asset.
	 * The AGX Dynamics Native will be created immediately.
	 */
	static UAGX_TrackProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_TrackProperties* Source);

	/**
	 * Get the instance, i.e. Play version, of this Track Properties.
	 *
	 * For an asset Track Properties GetInstance will return nullptr if we are not currently in Play
	 * or if an instance has not been created with GetOrCreateInstance yet.
	 *
	 * For an instance GetInstance will always return itself.
	 */
	UAGX_TrackProperties* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TrackProperties is a UAGX_TrackPropertiesAsset,
	 * returns a UAGX_TrackPropertiesInstance representing the TrackProperties asset throughout the
	 * lifetime of the GameInstance. If this is already a UAGX_TrackPropertiesInstance it returns
	 * itself. Returns null if not in-game (invalid call).
	 */
	UAGX_TrackProperties* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TrackProperties is a UAGX_TrackPropertiesInstance, returns the
	 * UAGX_TrackPropertiesAsset it was created from (if it still exists). Else returns null.
	 */
	UAGX_TrackProperties* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTrackPropertiesBarrier* GetNative();
	const FTrackPropertiesBarrier* GetNative() const;
	FTrackPropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	static constexpr double DefaultHingeCompliance = 1.0E-10;
	static constexpr double DefaultHingeSpookDamping = 2.0 / 60.0;

private:
	TWeakObjectPtr<UAGX_TrackProperties> Asset;
	TWeakObjectPtr<UAGX_TrackProperties> Instance;
	FTrackPropertiesBarrier NativeBarrier;
};
