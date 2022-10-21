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
 * Acts as an interface to a native AGX TrackProperties, and encapsulates it so that it is
 * completely hidden from code that includes this file.
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
	void SetHingeComplianceTranslationalX(double Compliance);
	void SetHingeComplianceTranslationalY(double Compliance);
	void SetHingeComplianceTranslationalZ(double Compliance);
	void SetHingeComplianceRotational(double Compliance);
	void SetHingeComplianceRotationalX(double Compliance);
	void SetHingeComplianceRotationalY(double Compliance);
	void SetHingeComplianceRotationalZ(double Compliance);
	double GetHingeCompliance(int32 DOF) const;

	void SetHingeSpookDamping(double Damping, int32 DOF);
	void SetHingeSpookDampingTranslational(double Damping);
	void SetHingeSpookDampingTranslationalX(double Damping);
	void SetHingeSpookDampingTranslationalY(double Damping);
	void SetHingeSpookDampingTranslationalZ(double Damping);
	void SetHingeSpookDampingRotational(double Damping);
	void SetHingeSpookDampingRotationalX(double Damping);
	void SetHingeSpookDampingRotationalY(double Damping);
	double GetHingeSpookDamping(int32 DOF) const;

	void SetHingeRangeEnabled(bool bEnable);
	bool GetHingeRangeEnabled() const;

	void SetHingeRangeRange(FAGX_RealInterval MinMaxAngles);
	FAGX_RealInterval GetHingeRangeRange() const;

	// Merge/Split properties.

	void SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable);
	bool GetOnInitializeMergeNodesToWheelsEnabled() const;

	void SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable);
	bool GetOnInitializeTransformNodesToWheelsEnabled() const;

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
