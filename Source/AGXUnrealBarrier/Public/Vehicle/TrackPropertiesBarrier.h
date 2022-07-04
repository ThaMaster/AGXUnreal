// Copyright 2022, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "NativeBarrier.h"
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// System includes.
#include <memory>


struct FTrackPropertiesRef;

/**
 * Acts as an interface to a native AGX TrackProperties, and encapsulates it so that it is completely
 * hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FTrackPropertiesBarrier
{
public:
	FTrackPropertiesBarrier();
	FTrackPropertiesBarrier(FTrackPropertiesBarrier&& Other);
	FTrackPropertiesBarrier(std::unique_ptr<FTrackPropertiesRef> Native);
	virtual ~FTrackPropertiesBarrier();

	bool HasNative() const;
	FTrackPropertiesRef* GetNative();
	const FTrackPropertiesRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

	FGuid GetGuid() const;

	// Hinge properties.

	void SetHingeCompliance(double Compliance, int32 DOF);
	void SetHingeComplianceTranslational(double Compliance);
	void SetHingeComplianceRotational(double Compliance);
	double GetHingeCompliance(int32 DOF) const;

	void SetHingeDamping(double Damping, int32 DOF);
	void SetHingeDampingTranslational(double Damping);
	void SetHingeDampingRotational(double Damping);
	double GetHingeDamping(int32 DOF) const;

	void SetEnableHingeRange(bool bEnable);
	bool GetEnableHingeRange() const;

	void SetHingeRangeRange(FAGX_RealInterval MinMaxAngles);
	FAGX_RealInterval GetHingeRangeRange() const;

	// Merge/Split properties.

	void SetEnableOnInitializeMergeNodesToWheels(bool bEnable);
	bool GetEnableOnInitializeMergeNodesToWheels() const;

	void SetEnableOnInitializeTransformNodesToWheels(bool bEnable);
	bool GetEnableOnInitializeTransformNodesToWheels() const;

	void SetTransformNodesToWheelsOverlap(double Overlap);
	double GetTransformNodesToWheelsOverlap() const;

	void SetNodesToWheelsMergeThreshold(double MergeThreshold);
	double GetNodesToWheelsMergeThreshold() const;

	void SetNodesToWheelsSplitThreshold(double SplitThreshold);
	double GetNodesToWheelsSplitThreshold() const;

	void SetNumNodesIncludedInAverageDirection(uint32 NumIncludedNodes);
	uint32 GetNumNodesIncludedInAverageDirection() const;

	// Stabilizing properties.

	void SetMinStabilizingHingeNormalForce(double MinNormalForce);
	double GetMinStabilizingHingeNormalForce() const;

	void SetStabilizingHingeFrictionParameter(double FrictionParameter);
	double GetStabilizingHingeFrictionParameter() const;

private:

	FTrackPropertiesBarrier(const FTrackPropertiesBarrier&) = delete;
	void operator=(const FTrackPropertiesBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTrackPropertiesRef> NativeRef;
};
